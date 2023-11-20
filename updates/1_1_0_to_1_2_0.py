from pymongo import MongoClient
from bson.objectid import ObjectId
import datetime
import argparse


parser = argparse.ArgumentParser(description='SCINE Database Update 1.1.X to 1.2.0.')
parser.add_argument('--ip', dest='ip', type=str, default='localhost',
                    help='The database server IP or hostname. [default = localhost]')
parser.add_argument('--port', dest='port', type=int, default=27017,
                    help='The database server port. [default = 27017]')
parser.add_argument('--name', dest='db_name', type=str, default='my_awesome_database',
                    help='The database name. [default = my_awesome_database]')
args = parser.parse_args()

ip = args.ip
port = args.port
db_name = args.db_name

client = MongoClient(ip, port)
db = client[db_name]

# Check version
is_correct_old_version = False
if "_db_meta_data" in db.list_collection_names():
    meta_data = db["_db_meta_data"].find_one({})
    version = meta_data["version"]
    if version["major"] == 1 and version["minor"] == 1:
        is_correct_old_version = True
else:
    is_correct_old_version = False

if not is_correct_old_version:
    print("The database does not have the correct old version for this update.")
    print("Exiting!")
    exit(1)

# Update content
# Add restart information to calculations
db["calculations"].update_many({}, {"$set": {"restart_information": {}}})

# Add pressure to model
db["calculations"].update_many({}, {"$set": {"model.pressure": "101325.0"}})
db["properties"].update_many({}, {"$set": {"model.pressure": "101325.0"}})
db["structures"].update_many({}, {"$set": {"model.pressure": "101325.0"}})


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
        "minor": 2,
        "patch": 0
    },
    "_created": date
}
_db_meta_data.insert_one(post)
