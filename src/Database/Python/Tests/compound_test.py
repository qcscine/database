# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

import scine_utilities as utils
import scine_database as db
import unittest
import os


class CompoundTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP') or '127.0.0.1'
        self.manager.credentials.database_name = "unittest_db_CompoundTest"
        self.manager.connect()
        self.manager.init()

    def tearDown(self):
        self.manager.wipe()

    def test_creation_one(self):
        coll = self.manager.get_collection("compounds")
        compound = db.Compound.make([db.ID(), db.ID()], coll)
        assert compound.has_id()

    def test_creation_two(self):
        coll = self.manager.get_collection("compounds")
        compound = db.Compound()
        compound.link(coll)
        comp = compound.create([db.ID()])
        assert compound.get_id() == comp

    def test_create_fails_collection(self):
        comp = db.Compound()
        self.assertRaises(RuntimeError, lambda: comp.create([db.ID()]))

    def test_centroid(self):
        coll = self.manager.get_collection("compounds")
        ref = db.ID()
        compound = db.Compound.make([ref, db.ID()], coll)
        assert ref == compound.get_centroid()

    def test_centroid_fails_collection(self):
        comp = db.Compound(db.ID())
        self.assertRaises(RuntimeError, lambda: comp.get_centroid())

    def test_centroid_fails_id(self):
        coll = self.manager.get_collection("compounds")
        comp = db.Compound()
        comp.link(coll)
        self.assertRaises(RuntimeError, lambda: comp.get_centroid())

    def test_centroid_fails_structures(self):
        coll = self.manager.get_collection("compounds")
        comp = db.Compound.make([], coll)
        self.assertRaises(RuntimeError, lambda: comp.get_centroid())

    def test_reactions(self):
        s1 = db.ID()
        s2 = db.ID()
        s3 = db.ID()
        coll = self.manager.get_collection("compounds")
        comp = db.Compound.make([s1, s2, s3], coll)
        assert comp.has_id()

        # Structure Functionalities
        r1 = db.ID()
        r2 = db.ID()
        r3 = db.ID()
        r4 = db.ID()
        r5 = db.ID()
        r6 = db.ID()

        reactions = comp.get_reactions()
        assert len(reactions) == 0
        comp.set_reactions([r1, r2, r3])
        reactions = comp.get_reactions()
        assert len(reactions) == 3
        assert reactions[0] == r1
        assert reactions[1] == r2
        assert reactions[2] == r3

        comp.add_reaction(r4)
        comp.remove_reaction(r1)
        reactions = comp.get_reactions()
        assert len(reactions) == 3
        assert reactions[0] == r2
        assert reactions[1] == r3
        assert reactions[2] == r4

        comp.set_reactions([r4, r5, r6])
        reactions = comp.get_reactions()
        assert len(reactions) == 3
        assert reactions[0] == r4
        assert reactions[1] == r5
        assert reactions[2] == r6
        assert comp.has_reaction(r4)

        comp.clear_reactions()
        reactions = comp.get_reactions()
        assert len(reactions) == 0
        assert not comp.has_reaction(r4)

    def test_reactions_fails_collection(self):
        comp = db.Compound()
        self.assertRaises(RuntimeError, lambda: comp.add_reaction(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.has_reaction(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.remove_reaction(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.set_reactions([]))
        self.assertRaises(RuntimeError, lambda: comp.get_reactions())
        self.assertRaises(RuntimeError, lambda: comp.clear_reactions())

    def test_reactions_fails_id(self):
        coll = self.manager.get_collection("compounds")
        comp = db.Compound()
        comp.link(coll)
        self.assertRaises(RuntimeError, lambda: comp.add_reaction(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.has_reaction(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.remove_reaction(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.set_reactions([]))
        self.assertRaises(RuntimeError, lambda: comp.get_reactions())
        self.assertRaises(RuntimeError, lambda: comp.clear_reactions())

    def test_structures(self):
        s1 = db.ID()
        s2 = db.ID()
        s3 = db.ID()
        coll = self.manager.get_collection("compounds")
        comp = db.Compound.make([s1, s2, s3], coll)
        assert comp.has_id()

        # Structure Functionalities
        s4 = db.ID()
        s5 = db.ID()
        s6 = db.ID()

        structures = comp.get_structures()
        assert len(structures) == 3
        assert structures[0] == s1
        assert structures[1] == s2
        assert structures[2] == s3

        comp.add_structure(s4)
        comp.remove_structure(s1)
        structures = comp.get_structures()
        assert len(structures) == 3
        assert structures[0] == s2
        assert structures[1] == s3
        assert structures[2] == s4

        comp.set_structures([s4, s5, s6])
        structures = comp.get_structures()
        assert len(structures) == 3
        assert structures[0] == s4
        assert structures[1] == s5
        assert structures[2] == s6
        assert comp.has_structure(s4)

        comp.clear_structures()
        structures = comp.get_structures()
        assert len(structures) == 0
        assert not comp.has_structure(s4)

    def test_structures_fails_collection(self):
        comp = db.Compound()
        self.assertRaises(RuntimeError, lambda: comp.add_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.has_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.remove_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.set_structures([]))
        self.assertRaises(RuntimeError, lambda: comp.get_structures())
        self.assertRaises(RuntimeError, lambda: comp.clear_structures())

    def test_structures_fails_id(self):
        coll = self.manager.get_collection("compounds")
        comp = db.Compound()
        comp.link(coll)
        self.assertRaises(RuntimeError, lambda: comp.add_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.has_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.remove_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: comp.set_structures([]))
        self.assertRaises(RuntimeError, lambda: comp.get_structures())
        self.assertRaises(RuntimeError, lambda: comp.clear_structures())
