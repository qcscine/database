from pymongo import MongoClient
from bson.objectid import ObjectId
import datetime
import argparse


parser = argparse.ArgumentParser(description='SCINE Database Update 1.0.X to 1.1.0.')
parser.add_argument('--ip', dest='ip', type=str, default='localhost',
                    help='The database server IP or hostname. [default = localhost]')
parser.add_argument('--port', dest='port', type=int, default=27017,
                    help='The database server port. [default = 27017]')
parser.add_argument('--name', dest='db_name', type=str, default='my_awesome_database',
                    help='The database name. [default = my_awesome_database]')
parser.add_argument('--duplicates', dest='duplicates', type=bool, default=False,
                    help='Option to extract duplicates. [default = False]')
args = parser.parse_args()

ip = args.ip
port = args.port
db_name = args.db_name
extract_duplicates = args.duplicates

client = MongoClient(ip, port)
db = client[db_name]

# Check version
is_correct_old_version = False
if "_db_meta_data" in db.list_collection_names():
    meta_data = db["_db_meta_data"].find_one({})
    version = meta_data["version"]
    if version["major"] == 1 and version["minor"] == 0:
        is_correct_old_version = True
else:
    is_correct_old_version = True

if not is_correct_old_version:
    print("The database does not have the correct old version for this update.")
    print("Exiting!")
    exit(1)

# Update content
# Add flask collection
if "flasks" not in db.list_collection_names():
    flasks = db["flasks"]
    flasks.create_index("flasks")
# Add type to reactants of reaction
for reaction in db["reactions"].find():
    if len(reaction['lhs']) == 0 and len(reaction['rhs']) == 0:
        continue
    elif len(reaction['lhs']) != 0:
        if isinstance(reaction['lhs'][0], type(dict)):
            if 'type' in reaction['lhs'][0]:
                continue
    elif len(reaction['rhs']) != 0:
        if isinstance(reaction['rhs'][0], type(dict)):
            if 'type' in reaction['rhs'][0]:
                continue
    lids = [lid for lid in reaction['lhs']]
    new_lhs = []
    for lid in lids:
        new_lhs.append({'id': lid, 'type': 'compound'})
    rids = [rid for rid in reaction['rhs']]
    new_rhs = []
    for rid in rids:
        new_rhs.append({'id': rid, 'type': 'compound'})
    db.reactions.update_one({'_id': reaction['_id']},
                            {'$set': {
                                'lhs': new_lhs,
                                'rhs': new_rhs
                            }}, upsert=False)

# Add 'duplicate_of' field to structure
if extract_duplicates:
    for structure in db['structures'].find({'label': 'duplicate'}):
        # Check, if comment has correct form
        if ' '.join(structure['comment'].split()[:-1]) == 'Structure is a duplicate of':
            # Extract ID of original from comment
            duplicate_id = structure['comment'].split()[-1][:-1]
            db.structures.update_one({'_id': structure['_id']},
                                     {"$set": {"duplicate_of": ObjectId(duplicate_id)}},
                                     upsert=False
                                     )
# Rename compound field to aggregate field
db["structures"].update_many({"compound": {"$exists": True}}, {"$rename": {"compound": "aggregate"}})


# Update version
if "_db_meta_data" in db.list_collection_names():
    _db_meta_data = db["_db_meta_data"]
    meta_data = db["_db_meta_data"].find_one({})
    date = meta_data["_created"]
    db["_db_meta_data"].delete_many({})
else:
    date = datetime.datetime.utcnow()
    _db_meta_data = db["_db_meta_data"]
post = {
    "version": {
        "major": 1,
        "minor": 1,
        "patch": 0
    },
    "_created": date
}
_db_meta_data.insert_one(post)
