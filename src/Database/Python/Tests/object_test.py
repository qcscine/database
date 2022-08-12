# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

import scine_utilities as utils
import scine_database as db
import unittest
import os
from time import sleep


class ObjectTest(unittest.TestCase):
    """
    In difference to the C++ unittests this version of the
    unittests for all the functions that stem from the
    object base class does not use a mock class but the
    NumberProperty instead.
    """

    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP', '127.0.0.1')
        self.manager.credentials.port = int(os.environ.get('TEST_MONGO_DB_PORT', 27017))
        self.manager.credentials.database_name = "unittest_db_ObjectTest"
        self.manager.connect()
        self.manager.init()

    def tearDown(self):
        self.manager.wipe()

    def test_linkage(self):
        coll = self.manager.get_collection("properties")
        obj = db.NumberProperty(db.ID())
        assert not obj.has_link()
        obj.link(coll)
        assert obj.has_link()
        assert obj.get_collection() == coll
        obj.detach()
        assert not obj.has_link()

    def test_wipe(self):
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        obj = db.NumberProperty.make("density_matrix", model, 42.12345, coll)
        assert obj.has_id()
        test_id = obj.get_id()
        assert coll.has(test_id)
        assert obj.exists()
        obj.wipe()
        assert not obj.has_id()
        assert not coll.has(test_id)

    def test_copy(self):
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        obj = db.NumberProperty(db.ID())
        copy1 = obj
        assert copy1.has_id()
        assert not copy1.has_link()
        obj = db.NumberProperty.make("density_matrix", model, 42.12345, coll)
        copy2 = obj
        assert copy2.has_id()
        assert copy2.has_link()

    def test_missing_collection(self):
        test = db.NumberProperty(db.ID())
        self.assertRaises(RuntimeError, lambda: test.get_collection())

    def test_missing_existance_failure_one(self):
        test = db.NumberProperty(db.ID())
        self.assertRaises(RuntimeError, lambda: test.exists())

    def test_json(self):
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        obj = db.NumberProperty.make("density_matrix", model, 42.12345, coll)
        assert obj.json() != ""

    def test_dates(self):
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        mock1 = db.NumberProperty.make("density_matrix", model, 42.12345, coll)
        sleep(1)
        mock2 = db.NumberProperty.make("density_matrix", model, 42.12345, coll)
        mock1.created()
        mock1.last_modified()
        assert mock1.has_created_timestamp()
        assert mock1.has_last_modified_timestamp()
        assert mock1.older_than(mock2, False)
        assert mock1.older_than(mock2, True)
        mock1.touch()
        assert mock2.older_than(mock1, True)

    def test_dates_fails_one(self):
        mock = db.NumberProperty(db.ID())
        self.assertRaises(RuntimeError, lambda: mock.created())
        self.assertRaises(RuntimeError, lambda: mock.has_created_timestamp())
        self.assertRaises(RuntimeError, lambda: mock.touch())
        coll = self.manager.get_collection("properties")
        mock.link(coll)
        self.assertRaises(RuntimeError, lambda: mock.created())

    def test_dates_fails_two(self):
        mock = db.NumberProperty(db.ID())
        self.assertRaises(RuntimeError, lambda: mock.last_modified())
        self.assertRaises(
            RuntimeError, lambda: mock.has_last_modified_timestamp())
        coll = self.manager.get_collection("properties")
        mock.link(coll)
        self.assertRaises(RuntimeError, lambda: mock.last_modified())

    def test_disabling_fails(self):
        mock = db.NumberProperty(db.ID())
        # Test exceptions
        self.assertRaises(RuntimeError, lambda: mock.explore())
        self.assertRaises(RuntimeError, lambda: mock.analyze())
        self.assertRaises(RuntimeError, lambda: mock.disable_analysis())
        self.assertRaises(RuntimeError, lambda: mock.disable_exploration())
        self.assertRaises(RuntimeError, lambda: mock.enable_analysis())
        self.assertRaises(RuntimeError, lambda: mock.enable_exploration())

    def test_disabling(self):
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        obj = db.NumberProperty.make("density_matrix", model, 42.12345, coll)
        # Test functionality
        assert obj.analyze()
        assert obj.explore()
        obj.disable_analysis()
        assert not obj.analyze()
        obj.disable_exploration()
        assert not obj.explore()
        obj.enable_analysis()
        assert obj.analyze()
        obj.enable_exploration()
        assert obj.explore()
