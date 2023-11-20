# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.
See LICENSE.txt for details.
"""

import scine_database as db
import unittest
import os


class StringPropertyTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP', '127.0.0.1')
        self.manager.credentials.port = int(os.environ.get('TEST_MONGO_DB_PORT', 27017))
        self.manager.credentials.database_name = "unittest_db_StringPropertyTest"
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
        test = db.StringProperty.make(
            "density_matrix", model, "Dummy", s, c, coll)
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
        assert data_db == "Dummy"

    def test_make_two(self):
        # Setup
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.StringProperty.make("density_matrix", model, "Dummy", coll)
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
        assert data_db == "Dummy"

    def test_create_one(self):
        # Setup
        coll = self.manager.get_collection("properties")
        c = db.ID()
        s = db.ID()
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.StringProperty()
        test.link(coll)
        test.create(model, "density_matrix", s, c, "Dummy")
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
        assert data_db == "Dummy"

    def test_create_two(self):
        # Setup
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.StringProperty()
        test.link(coll)
        test.create(model, "density_matrix", "Dummy")
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
        assert data_db == "Dummy"

    def test_data(self):
        # Setup
        coll = self.manager.get_collection("properties")
        s = db.ID()
        c = db.ID()
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.StringProperty.make(
            "density_matrix", model, "Dummy", s, c, coll)
        assert test.has_id()

        assert test.get_data() == "Dummy"
        test.set_data("Fake!")
        assert test.get_data() == "Fake!"

    def test_data_failure(self):
        # Setup
        coll = self.manager.get_collection("properties")
        test = db.StringProperty()
        self.assertRaises(RuntimeError, lambda: test.set_data("Dummy"))
        self.assertRaises(RuntimeError, lambda: test.get_data())
        test.link(coll)
        self.assertRaises(RuntimeError, lambda: test.set_data("Dummy"))
        self.assertRaises(RuntimeError, lambda: test.get_data())
