# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.
See LICENSE.txt for details.
"""

import scine_utilities as utils
import scine_database as db
import doctest
import sys
import os
from typing import Dict, Any


def db_testglobs() -> Dict[str, Any]:
    manager = db.Manager()
    manager.credentials.hostname = os.environ.get('TEST_MONGO_DB_IP', '127.0.0.1')
    manager.credentials.port = int(os.environ.get('TEST_MONGO_DB_PORT', 27017))
    manager.credentials.database_name = "scine-db-doctest"
    try:
        manager.wipe(True)  # Remote wipe to delete old dbs of the same name
        manager.connect()
        manager.init()
    except RuntimeError as e:
        print("Could not prepare a mongodb for doctesting: {}!".format(e))
        sys.exit(1)
    atoms = utils.AtomCollection(3)
    model = db.Model("dft", "pbe", "def2-svp")
    return locals()


if __name__ == "__main__":
    f, _ = doctest.testmod(db, extraglobs=db_testglobs())
    manager = db.Manager()
    manager.credentials.hostname = os.environ.get('TEST_MONGO_DB_IP', '127.0.0.1')
    manager.credentials.port = int(os.environ.get('TEST_MONGO_DB_PORT', 27017))
    manager.credentials.database_name = "scine-db-doctest"
    manager.wipe(True)
    sys.exit(f > 0)
