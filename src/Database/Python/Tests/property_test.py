# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.
See LICENSE.txt for details.
"""

import scine_database as db
import unittest
import os


class PropertyTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP', '127.0.0.1')
        self.manager.credentials.port = int(os.environ.get('TEST_MONGO_DB_PORT', 27017))
        self.manager.credentials.database_name = "unittest_db_PropertyTest"
        self.manager.connect()
        self.manager.init()

    def tearDown(self):
        self.manager.wipe()

    def test_property_name(self):
        # Setup
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.NumberProperty.make("density_matrix", model, 0.0, coll)
        prop = db.Property(test.id(), coll)
        assert prop.has_id()

        # PropertyName Functionalities
        assert prop.get_property_name() == "density_matrix"
        prop.set_property_name("electronic_energy")
        assert prop.get_property_name() == "electronic_energy"

    def test_property_name_fails_collection(self):
        prop = db.Property(db.ID())
        self.assertRaises(RuntimeError, lambda: prop.set_property_name("asdf"))
        self.assertRaises(RuntimeError, lambda: prop.get_property_name())

    def test_property_name_fails_id(self):
        coll = self.manager.get_collection("properties")
        prop = db.NumberProperty()
        prop.link(coll)
        self.assertRaises(RuntimeError, lambda: prop.set_property_name("asdf"))
        self.assertRaises(RuntimeError, lambda: prop.get_property_name())

    def test_model(self):
        # Setup
        coll = self.manager.get_collection("properties")

        model = db.Model("dft", "pbe", "def2-svp")
        prop = db.NumberProperty.make("density_matrix", model, 0.0, coll)
        assert prop.has_id()

        # Model Functionalities
        model = prop.get_model()
        assert model.method == "pbe"
        assert model.method_family == "dft"
        assert model.basis_set == "def2-svp"
        assert model.spin_mode == "any"
        model.basis_set = "b"
        model.spin_mode = "c"
        model.program = "d"
        model.version = "e"
        model.solvation = "f"
        model.solvent = "g"
        model.embedding = "h"
        model.periodic_boundaries = "i"
        model.external_field = "j"
        model.method_family = "k"
        prop.set_model(model)
        model_db = prop.get_model()
        assert model.spin_mode == model_db.spin_mode
        assert model.basis_set == model_db.basis_set
        assert model.program == model_db.program
        assert model.version == model_db.version
        assert model.solvation == model_db.solvation
        assert model.solvent == model_db.solvent
        assert model.embedding == model_db.embedding
        assert model.periodic_boundaries == model_db.periodic_boundaries
        assert model.external_field == model_db.external_field
        assert model.method_family == model_db.method_family

    def test_model_fails_collection(self):
        prop = db.Property(db.ID())
        model = db.Model("dft", "pbe", "def2-svp")
        self.assertRaises(RuntimeError, lambda: prop.set_model(model))
        self.assertRaises(RuntimeError, lambda: prop.get_model())

    def test_model_fails_id(self):
        coll = self.manager.get_collection("properties")
        prop = db.NumberProperty()
        prop.link(coll)
        model = db.Model("dft", "pbe", "def2-svp")
        self.assertRaises(RuntimeError, lambda: prop.set_model(model))
        self.assertRaises(RuntimeError, lambda: prop.get_model())

    def test_structure(self):
        # Setup
        coll = self.manager.get_collection("properties")

        model = db.Model("dft", "pbe", "def2-svp")
        prop = db.NumberProperty.make("density_matrix", model, 0.0, coll)
        assert prop.has_id()

        # Tests
        sid = db.ID()
        assert not prop.has_structure()
        prop.set_structure(sid)
        assert prop.has_structure()
        assert prop.get_structure() == sid
        prop.clear_structure()
        assert not prop.has_structure()

    def test_structure_fails_collection(self):
        prop = db.Property(db.ID())
        sid = db.ID()
        self.assertRaises(RuntimeError, lambda: prop.set_structure(sid))
        self.assertRaises(RuntimeError, lambda: prop.get_structure())
        self.assertRaises(RuntimeError, lambda: prop.has_structure())
        self.assertRaises(RuntimeError, lambda: prop.clear_structure())

    def test_structure_fails_id(self):
        coll = self.manager.get_collection("properties")
        prop = db.NumberProperty()
        prop.link(coll)
        sid = db.ID()
        self.assertRaises(RuntimeError, lambda: prop.set_structure(sid))
        self.assertRaises(RuntimeError, lambda: prop.get_structure())
        self.assertRaises(RuntimeError, lambda: prop.has_structure())
        self.assertRaises(RuntimeError, lambda: prop.clear_structure())

    def test_calculation(self):
        # Setup
        coll = self.manager.get_collection("properties")

        model = db.Model("dft", "pbe", "def2-svp")
        prop = db.NumberProperty.make("density_matrix", model, 0.0, coll)
        assert prop.has_id()

        # Tests
        sid = db.ID()
        assert not prop.has_calculation()
        prop.set_calculation(sid)
        assert prop.has_calculation()
        assert prop.get_calculation() == sid
        prop.clear_calculation()
        assert not prop.has_calculation()

    def test_calculation_fails_collection(self):
        prop = db.Property(db.ID())
        sid = db.ID()
        self.assertRaises(RuntimeError, lambda: prop.set_calculation(sid))
        self.assertRaises(RuntimeError, lambda: prop.get_calculation())
        self.assertRaises(RuntimeError, lambda: prop.has_calculation())
        self.assertRaises(RuntimeError, lambda: prop.clear_calculation())

    def test_calculation_fails_id(self):
        coll = self.manager.get_collection("properties")
        prop = db.NumberProperty()
        prop.link(coll)
        sid = db.ID()
        self.assertRaises(RuntimeError, lambda: prop.set_calculation(sid))
        self.assertRaises(RuntimeError, lambda: prop.get_calculation())
        self.assertRaises(RuntimeError, lambda: prop.has_calculation())
        self.assertRaises(RuntimeError, lambda: prop.clear_calculation())

    def test_comment(self):
        # Setup
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        prop = db.NumberProperty.make("density_matrix", model, 0.0, coll)
        assert prop.has_id()

        # Tests
        assert not prop.has_comment()
        comment = prop.get_comment()
        assert comment == ""

        prop.set_comment("wubba lubba dub dub")
        assert prop.has_comment()
        comment = prop.get_comment()
        assert comment == "wubba lubba dub dub"

        prop.clear_comment()
        assert not prop.has_comment()
        comment = prop.get_comment()
        assert comment == ""

    def test_comment_fails_collection(self):
        prop = db.Property(db.ID())
        self.assertRaises(RuntimeError, lambda: prop.set_comment("test"))
        self.assertRaises(RuntimeError, lambda: prop.get_comment())
        self.assertRaises(RuntimeError, lambda: prop.has_comment())
        self.assertRaises(RuntimeError, lambda: prop.clear_comment())

    def test_comment_fails_id(self):
        coll = self.manager.get_collection("properties")
        prop = db.NumberProperty()
        prop.link(coll)
        self.assertRaises(RuntimeError, lambda: prop.set_comment("test"))
        self.assertRaises(RuntimeError, lambda: prop.get_comment())
        self.assertRaises(RuntimeError, lambda: prop.has_comment())
        self.assertRaises(RuntimeError, lambda: prop.clear_comment())

    def test_get_derived_fails(self):
        # Setup
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        tmp = db.NumberProperty.make("density_matrix", model, 5.6, coll)
        prop = db.Property(tmp.get_id(), coll)
        assert prop.has_id()
        num_prop = prop.get_derived()
        assert num_prop.get_data() == 5.6

    def test_get_derived(self):
        # Setup
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        tmp = db.NumberProperty.make("density_matrix", model, 5.6, coll)
        prop = db.Property(tmp.get_id())
        self.assertRaises(RuntimeError, lambda: prop.get_derived())
        prop.link(coll)
        assert prop.has_id()
