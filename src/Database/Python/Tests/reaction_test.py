# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.
See LICENSE.txt for details.
"""

import scine_database as db
import unittest
import os


class ReactionTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP', '127.0.0.1')
        self.manager.credentials.port = int(os.environ.get('TEST_MONGO_DB_PORT', 27017))
        self.manager.credentials.database_name = "unittest_db_ReactionTest"
        self.manager.connect()
        self.manager.init()

    def tearDown(self):
        self.manager.wipe()

    def test_creation_one(self):
        coll = self.manager.get_collection("reactions")
        reaction = db.Reaction.make([db.ID()], [db.ID()], coll)
        assert reaction.has_id()

    def test_creation_two(self):
        coll = self.manager.get_collection("reactions")
        reaction = db.Reaction()
        reaction.link(coll)
        sid = reaction.create([db.ID()], [db.ID()])
        assert reaction.get_id() == sid

    def test_reactants_lhs(self):
        # Basic setup
        coll = self.manager.get_collection("reactions")
        id1 = db.ID()
        id2 = db.ID()
        reaction = db.Reaction.make([id1], [id2], coll, [db.CompoundOrFlask.COMPOUND], [db.CompoundOrFlask.FLASK])
        assert reaction.has_id()

        # Checks
        assert reaction.has_reactant(id1) == db.Side.LHS
        assert reaction.has_reactant(id2) == db.Side.RHS
        assert 1 == reaction.has_reactants()[0]
        assert 1 == reaction.has_reactants()[1]
        id3 = db.ID()
        id4 = db.ID()
        id5 = db.ID()
        id6 = db.ID()
        reaction.add_reactant(id3, db.Side.LHS, db.CompoundOrFlask.FLASK)
        assert reaction.has_reactant(id3) == db.Side.LHS
        assert 2 == reaction.has_reactants()[0]
        assert 1 == reaction.has_reactants()[1]
        reactants = reaction.get_reactants(db.Side.LHS)
        assert reaction.get_reactant_type(id3) == db.CompoundOrFlask.FLASK
        assert reactants[0][0], id1
        assert reactants[0][1], id3
        assert len(reactants[1]) == 0
        reaction.set_reactants([id4, id5, id6], db.Side.LHS)
        assert 3 == reaction.has_reactants()[0]
        reaction.remove_reactant(id5, db.Side.LHS)
        assert 2 == reaction.has_reactants()[0]
        reaction.clear_reactants(db.Side.LHS)
        assert 0 == reaction.has_reactants()[0]

    def test_reactants_rhs(self):
        # Basic setup
        coll = self.manager.get_collection("reactions")
        id1 = db.ID()
        id2 = db.ID()
        reaction = db.Reaction.make([id1], [id2], coll, [db.CompoundOrFlask.COMPOUND], [db.CompoundOrFlask.FLASK])
        assert reaction.has_id()

        # Checks
        assert reaction.has_reactant(id1) == db.Side.LHS
        assert reaction.has_reactant(id2) == db.Side.RHS
        assert 1 == reaction.has_reactants()[0]
        assert 1 == reaction.has_reactants()[1]
        id3 = db.ID()
        id4 = db.ID()
        id5 = db.ID()
        id6 = db.ID()
        reaction.add_reactant(id3, db.Side.RHS, db.CompoundOrFlask.FLASK)
        assert reaction.has_reactant(id3) == db.Side.RHS
        assert 1 == reaction.has_reactants()[0]
        assert 2 == reaction.has_reactants()[1]
        reactants = reaction.get_reactants(db.Side.RHS)
        assert reaction.get_reactant_type(id3) == db.CompoundOrFlask.FLASK
        assert reactants[1][0], id1
        assert reactants[1][1], id3
        assert len(reactants[0]) == 0
        reaction.set_reactants([id4, id5, id6], db.Side.RHS)
        assert 3 == reaction.has_reactants()[1]
        reaction.remove_reactant(id5, db.Side.RHS)
        assert 2 == reaction.has_reactants()[1]
        reaction.clear_reactants(db.Side.RHS)
        assert 0 == reaction.has_reactants()[1]

    def test_reactants_both(self):
        # Basic setup
        coll = self.manager.get_collection("reactions")
        id1 = db.ID()
        id2 = db.ID()
        reaction = db.Reaction.make([id1], [id2], coll, [db.CompoundOrFlask.COMPOUND], [db.CompoundOrFlask.FLASK])
        assert reaction.has_id()

        # Checks
        assert reaction.has_reactant(id1) == db.Side.LHS
        assert reaction.has_reactant(id2) == db.Side.RHS
        assert 1 == reaction.has_reactants()[0]
        assert 1 == reaction.has_reactants()[1]
        id3 = db.ID()
        id4 = db.ID()
        id5 = db.ID()
        id6 = db.ID()
        reaction.add_reactant(id3, db.Side.BOTH, db.CompoundOrFlask.FLASK)
        assert reaction.has_reactant(id3) == db.Side.BOTH
        assert reaction.get_reactant_type(id3) == db.CompoundOrFlask.FLASK
        assert 2 == reaction.has_reactants()[0]
        assert 2 == reaction.has_reactants()[1]
        reactants = reaction.get_reactants(db.Side.BOTH)
        assert reactants[0][0] == id1
        assert reactants[0][1] == id3
        assert reactants[1][0] == id2
        assert reactants[1][1] == id3
        reaction.set_reactants([id4, id5, id6], db.Side.BOTH)
        assert 3 == reaction.has_reactants()[0]
        assert 3 == reaction.has_reactants()[1]
        reaction.remove_reactant(id5, db.Side.BOTH)
        assert 2 == reaction.has_reactants()[0]
        assert 2 == reaction.has_reactants()[1]
        reaction.clear_reactants(db.Side.BOTH)
        assert 0 == reaction.has_reactants()[0]
        assert 0 == reaction.has_reactants()[1]

    def test_reactant_fails_collection(self):
        reaction = db.Reaction(db.ID())
        self.assertRaises(RuntimeError, lambda: reaction.has_reactant(db.ID()))
        self.assertRaises(RuntimeError, lambda: reaction.has_reactants())
        self.assertRaises(
            RuntimeError, lambda: reaction.get_reactants(db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: reaction.add_reactant(db.ID(), db.Side.BOTH, db.CompoundOrFlask.COMPOUND))
        self.assertRaises(
            RuntimeError, lambda: reaction.set_reactants([], db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: reaction.remove_reactant(db.ID(), db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: reaction.clear_reactants(db.Side.BOTH))

    def test_reactant_fails_id(self):
        coll = self.manager.get_collection("reactions")
        reaction = db.Reaction()
        reaction.link(coll)
        self.assertRaises(RuntimeError, lambda: reaction.has_reactant(db.ID()))
        self.assertRaises(RuntimeError, lambda: reaction.has_reactants())
        self.assertRaises(
            RuntimeError, lambda: reaction.get_reactants(db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: reaction.add_reactant(db.ID(), db.Side.BOTH, db.CompoundOrFlask.COMPOUND))
        self.assertRaises(
            RuntimeError, lambda: reaction.set_reactants([], db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: reaction.remove_reactant(db.ID(), db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: reaction.clear_reactants(db.Side.BOTH))

    def test_elementary_steps(self):
        # Basic setup
        coll = self.manager.get_collection("reactions")
        id1 = db.ID()
        id2 = db.ID()
        reaction = db.Reaction.make([id1], [id2], coll)
        assert reaction.has_id()
        # Checks
        assert not reaction.has_elementary_steps()
        assert not reaction.has_elementary_step(id1)
        reaction.add_elementary_step(id2)
        assert reaction.has_elementary_step(id2)
        assert 1 == reaction.has_elementary_steps()
        id3 = db.ID()
        id4 = db.ID()
        id5 = db.ID()
        reaction.set_elementary_steps([id3, id4, id5])
        assert 3 == reaction.has_elementary_steps()
        ret = reaction.get_elementary_steps()
        assert ret[0] == id3
        assert ret[1] == id4
        assert ret[2] == id5
        reaction.remove_elementary_step(id4)
        assert 2 == reaction.has_elementary_steps()
        assert not reaction.has_elementary_step(id4)
        reaction.clear_elementary_steps()
        assert 0 == reaction.has_elementary_steps()

    def test_elementary_step_fails_collection(self):
        reaction = db.Reaction(db.ID())
        self.assertRaises(
            RuntimeError, lambda: reaction.has_elementary_step(db.ID()))
        self.assertRaises(
            RuntimeError, lambda: reaction.has_elementary_steps())
        self.assertRaises(
            RuntimeError, lambda: reaction.get_elementary_steps())
        self.assertRaises(
            RuntimeError, lambda: reaction.add_elementary_step(db.ID()))
        self.assertRaises(
            RuntimeError, lambda: reaction.set_elementary_steps([]))
        self.assertRaises(
            RuntimeError, lambda: reaction.remove_elementary_step(db.ID()))
        self.assertRaises(
            RuntimeError, lambda: reaction.clear_elementary_steps())

    def test_elementary_step_fails_id(self):
        coll = self.manager.get_collection("reactions")
        reaction = db.Reaction()
        reaction.link(coll)
        self.assertRaises(
            RuntimeError, lambda: reaction.has_elementary_step(db.ID()))
        self.assertRaises(
            RuntimeError, lambda: reaction.has_elementary_steps())
        self.assertRaises(
            RuntimeError, lambda: reaction.get_elementary_steps())
        self.assertRaises(
            RuntimeError, lambda: reaction.add_elementary_step(db.ID()))
        self.assertRaises(
            RuntimeError, lambda: reaction.set_elementary_steps([]))
        self.assertRaises(
            RuntimeError, lambda: reaction.remove_elementary_step(db.ID()))
        self.assertRaises(
            RuntimeError, lambda: reaction.clear_elementary_steps())
