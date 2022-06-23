# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

import scine_utilities as utils
import scine_database as db
import unittest
import os


class NumberPropertyTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP') or '127.0.0.1'
        self.manager.credentials.database_name = "unittest_db_NumberPropertyTest"
        self.manager.connect()
        self.manager.init()

    def tearDown(self):
        self.manager.wipe()

    def test_make_one(self):
        # Setup
        coll = self.manager.get_collection("properties")
        c = db.ID()
        s = db.ID()
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.NumberProperty.make(
            "density_matrix", model, 42.12345, s, c, coll)
        assert test.has_id()

        # Check Fields
        model = test.get_model()
        comment = test.get_comment()
        name = test.get_property_name()
        calculation = test.get_calculation()
        structure = test.get_structure()
        data_db = test.get_data()

        assert model.method == "pbe"
        assert model.basis_set == "def2-svp"
        assert model.spin_mode == "any"
        assert comment == ""
        assert name == "density_matrix"
        assert calculation.string() == c.string()
        assert structure.string() == s.string()
        assert data_db == 42.12345

    def test_make_two(self):
        # Setup
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.NumberProperty.make("density_matrix", model, 42.12345, coll)
        assert test.has_id()

        # Check Fields
        model = test.get_model()
        comment = test.get_comment()
        name = test.get_property_name()
        data_db = test.get_data()

        assert model.method == "pbe"
        assert model.basis_set == "def2-svp"
        assert model.spin_mode == "any"
        assert comment == ""
        assert name == "density_matrix"
        assert not test.has_calculation()
        assert not test.has_structure()
        assert data_db == 42.12345

    def test_create_one(self):
        # Setup
        coll = self.manager.get_collection("properties")
        c = db.ID()
        s = db.ID()
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.NumberProperty()
        test.link(coll)
        test.create(model, "density_matrix", s, c, 42.12345)
        assert test.has_id()

        # Check Fields
        model = test.get_model()
        comment = test.get_comment()
        name = test.get_property_name()
        calculation = test.get_calculation()
        structure = test.get_structure()
        data_db = test.get_data()

        assert model.method == "pbe"
        assert model.basis_set == "def2-svp"
        assert model.spin_mode == "any"
        assert comment == ""
        assert name == "density_matrix"
        assert calculation.string() == c.string()
        assert structure.string() == s.string()
        assert data_db == 42.12345

    def test_create_two(self):
        # Setup
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.NumberProperty()
        test.link(coll)
        test.create(model, "density_matrix", 42.12345)
        assert test.has_id()

        # Check Fields
        model = test.get_model()
        comment = test.get_comment()
        name = test.get_property_name()
        data_db = test.get_data()

        assert model.method == "pbe"
        assert model.basis_set == "def2-svp"
        assert model.spin_mode == "any"
        assert comment == ""
        assert name == "density_matrix"
        assert not test.has_calculation()
        assert not test.has_structure()
        assert data_db == 42.12345

    def test_data(self):
        # Setup
        coll = self.manager.get_collection("properties")
        s = db.ID()
        c = db.ID()
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.NumberProperty.make(
            "density_matrix", model, 42.12345, s, c, coll)
        assert test.has_id()

        assert test.get_data() == 42.12345
        test.set_data(12.345678)
        assert test.get_data() == 12.345678

    def test_data_failure(self):
        # Setup
        coll = self.manager.get_collection("properties")
        test = db.NumberProperty()
        self.assertRaises(RuntimeError, lambda: test.set_data(42.12345))
        self.assertRaises(RuntimeError, lambda: test.get_data())
        test.link(coll)
        self.assertRaises(RuntimeError, lambda: test.set_data(42.12345))
        self.assertRaises(RuntimeError, lambda: test.get_data())
