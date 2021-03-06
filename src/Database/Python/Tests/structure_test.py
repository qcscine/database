# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

import scine_utilities as utils
import scine_database as db
import unittest
import os


class StructureTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP') or '127.0.0.1'
        self.manager.credentials.database_name = "unittest_db_StructureTest"
        self.manager.connect()
        self.manager.init()
        self.atoms = utils.AtomCollection(
            [utils.ElementType.H, utils.ElementType.H], [
                [+1.0, 0.0, 0.0],
                [-1.0, 0.0, 0.0]
            ]
        )

    def tearDown(self):
        self.manager.wipe()

    def test_creation_one(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(self.atoms, 0, 1, coll)
        assert structure.has_id()

    def test_creation_two(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        structure.link(coll)
        structure.create(self.atoms, 0, 1)
        assert structure.has_id()

    def test_atoms(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()

        # Checks
        atoms2 = utils.AtomCollection(
            [
                utils.ElementType.He,
                utils.ElementType.Ar,
                utils.ElementType.Mo,
                utils.ElementType.Te
            ], [
                [1.0, 2.0, 3.0],
                [4.0, 5.0, 6.0],
                [0.0, 0.0, 0.0],
                [9.0, 9.0, 9.0]
            ]
        )
        assert 2 == structure.has_atoms()
        ret1 = structure.get_atoms()
        assert ret1.get_element(0) == utils.ElementType.H
        assert ret1.get_element(1) == utils.ElementType.H
        structure.set_atoms(atoms2)
        assert 4 == structure.has_atoms()
        ret2 = structure.get_atoms()
        assert ret2.get_element(0) == utils.ElementType.He
        assert ret2.get_element(1) == utils.ElementType.Ar
        assert ret2.get_element(2) == utils.ElementType.Mo
        assert ret2.get_element(3) == utils.ElementType.Te
        structure.clear_atoms()
        assert 0 == structure.has_atoms()

    def test_atoms_fail_collection(self):
        structure = db.Structure(db.ID())
        self.assertRaises(
            RuntimeError, lambda: structure.set_atoms(self.atoms))
        self.assertRaises(RuntimeError, lambda: structure.get_atoms())
        self.assertRaises(RuntimeError, lambda: structure.has_atoms())
        self.assertRaises(RuntimeError, lambda: structure.clear_atoms())

    def test_atoms_fail_id(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        structure.link(coll)
        self.assertRaises(
            RuntimeError, lambda: structure.set_atoms(self.atoms))
        self.assertRaises(RuntimeError, lambda: structure.get_atoms())
        self.assertRaises(RuntimeError, lambda: structure.has_atoms())
        self.assertRaises(RuntimeError, lambda: structure.clear_atoms())

    def test_charge(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()
        assert 0 == structure.get_charge()
        structure.set_charge(5)
        assert 5 == structure.get_charge()
        structure.set_charge(-999)
        assert -999 == structure.get_charge()

    def test_charge_fails_collection(self):
        structure = db.Structure(db.ID())
        self.assertRaises(RuntimeError, lambda: structure.set_charge(1))
        self.assertRaises(RuntimeError, lambda: structure.get_charge())

    def test_charge_fails_id(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        structure.link(coll)
        self.assertRaises(RuntimeError, lambda: structure.set_charge(1))
        self.assertRaises(RuntimeError, lambda: structure.get_charge())

    def test_multiplicity(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()
        assert 1 == structure.get_multiplicity()
        structure.set_multiplicity(5)
        assert 5 == structure.get_multiplicity()
        structure.set_multiplicity(-999)
        assert -999 == structure.get_multiplicity()

    def test_multiplicity_fails_collection(self):
        structure = db.Structure(db.ID())
        self.assertRaises(RuntimeError, lambda: structure.set_multiplicity(1))
        self.assertRaises(RuntimeError, lambda: structure.get_multiplicity())

    def test_multiplicity_fails_id(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        structure.link(coll)
        self.assertRaises(RuntimeError, lambda: structure.set_multiplicity(1))
        self.assertRaises(RuntimeError, lambda: structure.get_multiplicity())

    def test_model(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()

        # Check
        ret1 = structure.get_model()
        assert ret1.basis_set == model.basis_set
        assert ret1.temperature == model.temperature
        assert ret1.electronic_temperature == model.electronic_temperature
        assert ret1.method == model.method
        assert ret1.program == model.program
        model2 = db.Model("am1", "am1", "")
        model2.temperature = "278.0"
        model2.electronic_temperature = "5.0"
        model2.program = "sparrow"
        structure.set_model(model2)
        ret2 = structure.get_model()
        assert ret2.basis_set == model2.basis_set
        assert ret2.temperature == model2.temperature
        assert ret2.electronic_temperature == model2.electronic_temperature
        assert ret2.method == model2.method
        assert ret2.program == model2.program

    def test_model_fails_collection(self):
        structure = db.Structure(db.ID())
        model = db.Model("am1", "am1", "")
        self.assertRaises(RuntimeError, lambda: structure.set_model(model))
        self.assertRaises(RuntimeError, lambda: structure.get_model())

    def test_model_fails_id(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        structure.link(coll)
        model = db.Model("am1", "am1", "")
        self.assertRaises(RuntimeError, lambda: structure.set_model(model))
        self.assertRaises(RuntimeError, lambda: structure.get_model())

    def test_label(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()

        # Check
        assert db.Label.MINIMUM_GUESS == structure.get_label()
        structure.set_label(db.Label.NONE)
        assert db.Label.NONE == structure.get_label()
        structure.set_label(db.Label.TS_GUESS)
        assert db.Label.TS_GUESS == structure.get_label()

    def test_label_fails_collection(self):
        structure = db.Structure(db.ID())
        self.assertRaises(
            RuntimeError, lambda: structure.set_label(db.Label.USER_GUESS))
        self.assertRaises(RuntimeError, lambda: structure.get_label())

    def test_label_fails_id(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        structure.link(coll)
        self.assertRaises(
            RuntimeError, lambda: structure.set_label(db.Label.USER_GUESS))
        self.assertRaises(RuntimeError, lambda: structure.get_label())

    def test_compound(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()

        # Check
        assert not structure.has_compound()
        id2 = db.ID()
        structure.set_compound(id2)
        assert structure.has_compound()
        assert id2 == structure.get_compound()
        structure.clear_compound()
        assert not structure.has_compound()

    def test_compound_fails_collection(self):
        structure = db.Structure(db.ID())
        self.assertRaises(
            RuntimeError, lambda: structure.set_compound(db.ID()))
        self.assertRaises(RuntimeError, lambda: structure.get_compound())
        self.assertRaises(RuntimeError, lambda: structure.has_compound())
        self.assertRaises(RuntimeError, lambda: structure.clear_compound())

    def test_compound_fails_id(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        structure.link(coll)
        self.assertRaises(
            RuntimeError, lambda: structure.set_compound(db.ID()))
        self.assertRaises(RuntimeError, lambda: structure.get_compound())
        self.assertRaises(RuntimeError, lambda: structure.has_compound())
        self.assertRaises(RuntimeError, lambda: structure.clear_compound())

    def test_graph(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()

        # Check
        assert 0 == structure.has_graphs()
        assert not structure.has_graph("FROWN")
        structure.set_graph("FROWN", "HHC=CHH")
        assert 1 == structure.has_graphs()
        assert structure.has_graph("FROWN")
        assert "HHC=CHH" == structure.get_graph("FROWN")
        structure.remove_graph("FROWN")
        assert 0 == structure.has_graphs()
        assert not structure.has_graph("FROWN")
        graphs = {
            "FROWN": "ABCDEF",
            "SMILES": "FEDCBA"
        }
        structure.set_graphs(graphs)
        assert 2 == structure.has_graphs()
        assert structure.has_graph("FROWN")
        ret = structure.get_graphs()
        assert ret["FROWN"] == graphs["FROWN"]
        assert ret["SMILES"] == graphs["SMILES"]
        structure.clear_graphs()
        assert 0 == structure.has_graphs()
        assert not structure.has_graph("FROWN")

    def test_graph_fails_collection(self):
        structure = db.Structure(db.ID())
        self.assertRaises(
            RuntimeError, lambda: structure.set_graph("frown", "HHC=CHH"))
        self.assertRaises(RuntimeError, lambda: structure.get_graph("frown"))
        self.assertRaises(RuntimeError, lambda: structure.has_graph("frown"))
        self.assertRaises(RuntimeError, lambda: structure.set_graphs({}))
        self.assertRaises(RuntimeError, lambda: structure.get_graphs())
        self.assertRaises(RuntimeError, lambda: structure.has_graphs())
        self.assertRaises(
            RuntimeError, lambda: structure.remove_graph("frown"))
        self.assertRaises(RuntimeError, lambda: structure.clear_graphs())

    def test_graph_fails_id(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        structure.link(coll)
        self.assertRaises(
            RuntimeError, lambda: structure.set_graph("frown", "HHC=CHH"))
        self.assertRaises(RuntimeError, lambda: structure.get_graph("frown"))
        self.assertRaises(RuntimeError, lambda: structure.has_graph("frown"))
        self.assertRaises(RuntimeError, lambda: structure.set_graphs({}))
        self.assertRaises(RuntimeError, lambda: structure.get_graphs())
        self.assertRaises(RuntimeError, lambda: structure.has_graphs())
        self.assertRaises(
            RuntimeError, lambda: structure.remove_graph("frown"))
        self.assertRaises(RuntimeError, lambda: structure.clear_graphs())

    def test_comment(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()

        # Check
        assert not structure.has_comment()
        structure.set_comment("foo")
        assert structure.has_comment()
        assert "foo" == structure.get_comment()
        structure.clear_comment()
        assert not structure.has_comment()

    def test_comment_fails_collection(self):
        structure = db.Structure(db.ID())
        self.assertRaises(RuntimeError, lambda: structure.set_comment(""))
        self.assertRaises(RuntimeError, lambda: structure.get_comment())
        self.assertRaises(RuntimeError, lambda: structure.has_comment())
        self.assertRaises(RuntimeError, lambda: structure.clear_comment())

    def test_comment_fails_id(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        structure.link(coll)
        self.assertRaises(RuntimeError, lambda: structure.set_comment(""))
        self.assertRaises(RuntimeError, lambda: structure.get_comment())
        self.assertRaises(RuntimeError, lambda: structure.has_comment())
        self.assertRaises(RuntimeError, lambda: structure.clear_comment())

    def test_property_one(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()

        # Check
        id1 = db.ID()
        id2 = db.ID()
        assert not structure.has_property("hessian")
        assert not structure.has_property(id1)
        structure.set_property("hessian", id1)
        assert structure.has_property("hessian")
        assert structure.has_property(id1)
        assert not structure.has_property(id2)
        assert id1 == structure.get_property("hessian")
        structure.add_property("hessian", id2)
        assert structure.has_property(id2)
        self.assertRaises(
            RuntimeError, lambda: structure.get_property("hessian"))
        structure.remove_property("hessian", id1)
        assert structure.has_property(id2)
        assert not structure.has_property(id1)

    def test_property_two(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()

        # Check
        id1 = db.ID()
        id2 = db.ID()
        id3 = db.ID()
        assert 0 == structure.has_properties("hessian")
        structure.set_properties("hessian", [id1, id2])
        assert 2 == structure.has_properties("hessian")
        assert id1 == structure.get_properties("hessian")[0]
        assert id2 == structure.get_properties("hessian")[1]
        structure.set_properties("hessian", [id3])
        assert id3 == structure.get_properties("hessian")[0]
        assert 1 == structure.has_properties("hessian")
        structure.clear_properties("hessian")
        assert 0 == structure.has_properties("hessian")

    def test_property_three(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        coll = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, coll)
        assert structure.has_id()

        # Check
        prop1 = {
            "electronic_energy": [db.ID(), db.ID(), db.ID()],
            "hessian": [db.ID(), db.ID()],
            "gradients": [db.ID(), db.ID()],
        }
        prop2 = {
            "mulliken_charges": [db.ID(), db.ID()],
            "gradients": [db.ID(), db.ID()],
        }
        assert 0 == structure.has_properties("electronic_energy")
        assert 0 == structure.has_properties("mulliken_charges")
        assert 0 == structure.has_properties("hessian")
        assert 0 == structure.has_properties("gradients")
        structure.set_all_properties(prop1)
        ret1 = structure.get_all_properties()
        assert prop1["electronic_energy"][0] == ret1["electronic_energy"][0]
        assert prop1["electronic_energy"][2] == ret1["electronic_energy"][2]
        assert prop1["hessian"][0] == ret1["hessian"][0]
        assert prop1["hessian"][1] == ret1["hessian"][1]
        assert prop1["gradients"][0] == ret1["gradients"][0]
        structure.set_all_properties(prop2)
        ret2 = structure.get_all_properties()
        assert prop2["mulliken_charges"][0] == ret2["mulliken_charges"][0]
        assert prop2["mulliken_charges"][1] == ret2["mulliken_charges"][1]
        assert prop2["gradients"][0] == ret2["gradients"][0]
        assert prop2["gradients"][1] == ret2["gradients"][1]
        structure.clear_all_properties()
        assert 0 == structure.has_properties("electronic_energy")
        assert 0 == structure.has_properties("mulliken_charges")
        assert 0 == structure.has_properties("hessian")
        assert 0 == structure.has_properties("gradients")

    def test_property_four(self):
        # Setup
        model = db.Model("dft", "pbe", "def2-svp")
        structures = self.manager.get_collection("structures")
        structure = db.Structure.make(
            self.atoms, 0, 1, model, db.Label.MINIMUM_GUESS, structures)
        assert structure.has_id()
        model1 = db.Model("dft", "pbe", "def2-svp")
        model1.program = "serenity"
        model2 = db.Model("am1", "am1", "")
        model2.program = "sparrow"
        properties = self.manager.get_collection("properties")
        sid = structure.get_id()
        cid = db.ID()
        p1 = db.NumberProperty.make(
            "electronic_energy", model1, 1.0, sid, cid, properties)
        p2 = db.NumberProperty.make(
            "electronic_energy", model2, 2.0, sid, cid, properties)
        p3 = db.NumberProperty.make(
            "electronic_energy", model2, 3.0, sid, cid, properties)
        structure.add_property("electronic_energy", p1.id())
        structure.add_property("electronic_energy", p2.id())
        structure.add_property("electronic_energy", p3.id())

        # Checks
        none = db.Model("dft", "sdfasdf", "asdf")
        ret0 = structure.query_properties(
            "electronic_energy", none, properties)
        assert 0 == len(ret0)
        ret1 = structure.query_properties(
            "electronic_energy", model1, properties)
        assert 1 == len(ret1)
        ret2 = structure.query_properties(
            "electronic_energy", model2, properties)
        assert 2 == len(ret2)
        any_dft = db.Model("dft", "any", "any")
        ret3 = structure.query_properties(
            "electronic_energy", any_dft, properties)
        assert 1 == len(ret3)
        any_m = db.Model("any", "any", "any")
        ret4 = structure.query_properties(
            "electronic_energy", any_m, properties)
        assert 1 == len(ret4)
        ret5 = structure.query_properties(
            "none_existing_key", none, properties)
        assert 0 == len(ret5)

    def test_property_fails_collection(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        model = db.Model("dft", "pbe", "def2-svp")
        self.assertRaises(RuntimeError, lambda: structure.has_property("key"))
        self.assertRaises(
            RuntimeError, lambda: structure.has_property(db.ID()))
        self.assertRaises(RuntimeError, lambda: structure.get_property("key"))
        self.assertRaises(
            RuntimeError, lambda: structure.set_property("key", db.ID()))
        self.assertRaises(
            RuntimeError, lambda: structure.add_property("key", db.ID()))
        self.assertRaises(
            RuntimeError, lambda: structure.remove_property("key", db.ID()))
        self.assertRaises(
            RuntimeError, lambda: structure.set_properties("key", []))
        self.assertRaises(
            RuntimeError, lambda: structure.get_properties("key"))
        self.assertRaises(
            RuntimeError, lambda: structure.has_properties("key"))
        self.assertRaises(
            RuntimeError, lambda: structure.clear_properties("key"))
        self.assertRaises(
            RuntimeError, lambda: structure.query_properties("key", model, coll))
        self.assertRaises(RuntimeError, lambda: structure.get_all_properties())
        self.assertRaises(
            RuntimeError, lambda: structure.set_all_properties({"key": [db.ID()]}))
        self.assertRaises(
            RuntimeError, lambda: structure.clear_all_properties())

    def test_property_fails_id(self):
        coll = self.manager.get_collection("structures")
        structure = db.Structure()
        structure.link(coll)
        model = db.Model("dft", "pbe", "def2-svp")
        self.assertRaises(RuntimeError, lambda: structure.has_property("key"))
        self.assertRaises(
            RuntimeError, lambda: structure.has_property(db.ID()))
        self.assertRaises(RuntimeError, lambda: structure.get_property("key"))
        self.assertRaises(
            RuntimeError, lambda: structure.set_property("key", db.ID()))
        self.assertRaises(
            RuntimeError, lambda: structure.add_property("key", db.ID()))
        self.assertRaises(
            RuntimeError, lambda: structure.remove_property("key", db.ID()))
        self.assertRaises(
            RuntimeError, lambda: structure.set_properties("key", []))
        self.assertRaises(
            RuntimeError, lambda: structure.get_properties("key"))
        self.assertRaises(
            RuntimeError, lambda: structure.has_properties("key"))
        self.assertRaises(
            RuntimeError, lambda: structure.clear_properties("key"))
        self.assertRaises(
            RuntimeError, lambda: structure.query_properties("key", model, coll))
        self.assertRaises(RuntimeError, lambda: structure.get_all_properties())
        self.assertRaises(
            RuntimeError, lambda: structure.set_all_properties({"key": [db.ID()]}))
        self.assertRaises(
            RuntimeError, lambda: structure.clear_all_properties())
