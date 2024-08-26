#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__copyright__ = """ This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.
See LICENSE.txt for details.
"""
from pymongo import MongoClient
import datetime
import argparse


parser = argparse.ArgumentParser(description='SCINE Database Update 1.2.X to 1.3.0.')
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

client: MongoClient = MongoClient(ip, port)
db = client[db_name]

# Check version
is_correct_old_version = False
if "_db_meta_data" in db.list_collection_names():
    meta_data = db["_db_meta_data"].find_one({})
    assert meta_data
    version = meta_data["version"]
    if version["major"] == 1 and version["minor"] == 2:
        is_correct_old_version = True
else:
    is_correct_old_version = False

if not is_correct_old_version:
    print("The database does not have the correct old version for this update.")
    print("Exiting!")
    exit(1)

# Check for model_transformation steps
step = db["elementary_steps"].find_one({"type": "model_transformation"})
if step is not None:
    print("The database contains outdated elementary steps of type model_transformation.")
    print("These steps must be removed manually before updating")
    exit(1)

# Update version
if "_db_meta_data" in db.list_collection_names():
    _db_meta_data = db["_db_meta_data"]
    meta_data = db["_db_meta_data"].find_one({})
    assert meta_data
    date = meta_data["_created"]
    db["_db_meta_data"].delete_many({})
else:
    date = datetime.datetime.utcnow()
    _db_meta_data = db["_db_meta_data"]
post = {
    "version": {
        "major": 1,
        "minor": 3,
        "patch": 0
    },
    "_created": date
}
_db_meta_data.insert_one(post)
