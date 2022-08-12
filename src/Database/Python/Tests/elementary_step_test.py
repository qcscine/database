# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

import scine_utilities as utils
import scine_database as db
import unittest
import numpy as np
import os


class ElementaryStepTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP', '127.0.0.1')
        self.manager.credentials.port = int(os.environ.get('TEST_MONGO_DB_PORT', 27017))
        self.manager.credentials.database_name = "unittest_db_ElementaryStepTest"
        self.manager.connect()
        self.manager.init()

    def tearDown(self):
        self.manager.wipe()

    def test_creation_one(self):
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep.make([db.ID()], [db.ID()], coll)
        assert step.has_id()

    def test_creation_two(self):
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep()
        step.link(coll)
        sid = step.create([db.ID()], [db.ID()])
        assert step.get_id() == sid

    def test_create_fails_collection(self):
        step = db.ElementaryStep()
        self.assertRaises(
            RuntimeError, lambda: step.create([db.ID()], [db.ID()]))

    def test_reactants_lhs(self):
        # Basic setup
        coll = self.manager.get_collection("elementary_steps")
        id1 = db.ID()
        id2 = db.ID()
        step = db.ElementaryStep.make([id1], [id2], coll)
        assert step.has_id()

        # Checks
        assert step.has_reactant(id1) == db.Side.LHS
        assert step.has_reactant(id2) == db.Side.RHS
        assert 1 == step.has_reactants()[0]
        assert 1 == step.has_reactants()[1]
        id3 = db.ID()
        id4 = db.ID()
        id5 = db.ID()
        id6 = db.ID()
        step.add_reactant(id3, db.Side.LHS)
        assert step.has_reactant(id3) == db.Side.LHS
        assert 2 == step.has_reactants()[0]
        assert 1 == step.has_reactants()[1]
        reactants = step.get_reactants(db.Side.LHS)
        assert reactants[0][0], id1
        assert reactants[0][1], id3
        assert len(reactants[1]) == 0
        step.set_reactants([id4, id5, id6], db.Side.LHS)
        assert 3 == step.has_reactants()[0]
        step.remove_reactant(id5, db.Side.LHS)
        assert 2 == step.has_reactants()[0]
        step.clear_reactants(db.Side.LHS)
        assert 0 == step.has_reactants()[0]

    def test_reactants_rhs(self):
        # Basic setup
        coll = self.manager.get_collection("elementary_steps")
        id1 = db.ID()
        id2 = db.ID()
        step = db.ElementaryStep.make([id1], [id2], coll)
        assert step.has_id()

        # Checks
        assert step.has_reactant(id1) == db.Side.LHS
        assert step.has_reactant(id2) == db.Side.RHS
        assert 1 == step.has_reactants()[0]
        assert 1 == step.has_reactants()[1]
        id3 = db.ID()
        id4 = db.ID()
        id5 = db.ID()
        id6 = db.ID()
        step.add_reactant(id3, db.Side.RHS)
        assert step.has_reactant(id3) == db.Side.RHS
        assert 1 == step.has_reactants()[0]
        assert 2 == step.has_reactants()[1]
        reactants = step.get_reactants(db.Side.RHS)
        assert reactants[1][0], id1
        assert reactants[1][1], id3
        assert len(reactants[0]) == 0
        step.set_reactants([id4, id5, id6], db.Side.RHS)
        assert 3 == step.has_reactants()[1]
        step.remove_reactant(id5, db.Side.RHS)
        assert 2 == step.has_reactants()[1]
        step.clear_reactants(db.Side.RHS)
        assert 0 == step.has_reactants()[1]

    def test_reactants_both(self):
        # Basic setup
        coll = self.manager.get_collection("elementary_steps")
        id1 = db.ID()
        id2 = db.ID()
        step = db.ElementaryStep.make([id1], [id2], coll)
        assert step.has_id()

        # Checks
        assert step.has_reactant(id1) == db.Side.LHS
        assert step.has_reactant(id2) == db.Side.RHS
        assert 1 == step.has_reactants()[0]
        assert 1 == step.has_reactants()[1]
        id3 = db.ID()
        id4 = db.ID()
        id5 = db.ID()
        id6 = db.ID()
        step.add_reactant(id3, db.Side.BOTH)
        assert step.has_reactant(id3) == db.Side.BOTH
        assert 2 == step.has_reactants()[0]
        assert 2 == step.has_reactants()[1]
        reactants = step.get_reactants(db.Side.BOTH)
        assert reactants[0][0] == id1
        assert reactants[0][1] == id3
        assert reactants[1][0] == id2
        assert reactants[1][1] == id3
        step.set_reactants([id4, id5, id6], db.Side.BOTH)
        assert 3 == step.has_reactants()[0]
        assert 3 == step.has_reactants()[1]
        step.remove_reactant(id5, db.Side.BOTH)
        assert 2 == step.has_reactants()[0]
        assert 2 == step.has_reactants()[1]
        step.clear_reactants(db.Side.BOTH)
        assert 0 == step.has_reactants()[0]
        assert 0 == step.has_reactants()[1]

    def test_reactant_fails_collection(self):
        step = db.ElementaryStep(db.ID())
        self.assertRaises(RuntimeError, lambda: step.has_reactant(db.ID()))
        self.assertRaises(RuntimeError, lambda: step.has_reactants())
        self.assertRaises(
            RuntimeError, lambda: step.get_reactants(db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: step.add_reactant(db.ID(), db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: step.set_reactants([], db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: step.remove_reactant(db.ID(), db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: step.clear_reactants(db.Side.BOTH))

    def test_reactant_fails_id(self):
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep()
        step.link(coll)
        self.assertRaises(RuntimeError, lambda: step.has_reactant(db.ID()))
        self.assertRaises(RuntimeError, lambda: step.has_reactants())
        self.assertRaises(
            RuntimeError, lambda: step.get_reactants(db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: step.add_reactant(db.ID(), db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: step.set_reactants([], db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: step.remove_reactant(db.ID(), db.Side.BOTH))
        self.assertRaises(
            RuntimeError, lambda: step.clear_reactants(db.Side.BOTH))

    def test_type(self):
        # Basic setup
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep.make([db.ID()], [db.ID()], coll)
        assert step.has_id()
        # Checks
        assert db.ElementaryStepType.REGULAR, step.get_type()
        step.set_type(db.ElementaryStepType.BARRIERLESS)
        assert db.ElementaryStepType.BARRIERLESS == step.get_type()
        assert db.ElementaryStepType.REGULAR != step.get_type()

    def test_type_fails_collection(self):
        step = db.ElementaryStep(db.ID())
        self.assertRaises(RuntimeError, lambda: step.get_type())
        self.assertRaises(RuntimeError, lambda: step.set_type(
            db.ElementaryStepType.REGULAR))

    def test_type_fails_id(self):
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep()
        step.link(coll)
        self.assertRaises(RuntimeError, lambda: step.get_type())
        self.assertRaises(RuntimeError, lambda: step.set_type(
            db.ElementaryStepType.REGULAR))

    def test_transition_state(self):
        # Basic setup
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep.make([db.ID()], [db.ID()], coll)
        assert step.has_id()
        # Checks
        assert not step.has_transition_state()
        id3 = db.ID()
        step.set_transition_state(id3)
        assert step.has_transition_state()
        assert step.get_transition_state() == id3
        step.clear_transition_state()
        assert not step.has_transition_state()

    def test_transition_state_fails_collection(self):
        step = db.ElementaryStep(db.ID())
        self.assertRaises(RuntimeError, lambda: step.has_transition_state())
        self.assertRaises(RuntimeError, lambda: step.get_transition_state())
        self.assertRaises(
            RuntimeError, lambda: step.set_transition_state(db.ID()))
        self.assertRaises(RuntimeError, lambda: step.clear_transition_state())

    def test_transition_state_fails_id(self):
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep()
        step.link(coll)
        self.assertRaises(RuntimeError, lambda: step.has_transition_state())
        self.assertRaises(RuntimeError, lambda: step.get_transition_state())
        self.assertRaises(
            RuntimeError, lambda: step.set_transition_state(db.ID()))
        self.assertRaises(RuntimeError, lambda: step.clear_transition_state())

    def test_reaction(self):
        # Basic setup
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep.make([db.ID()], [db.ID()], coll)
        assert step.has_id()
        # Checks
        assert not step.has_reaction()
        id3 = db.ID()
        step.set_reaction(id3)
        assert step.has_reaction()
        assert step.get_reaction() == id3
        step.clear_reaction()
        assert not step.has_reaction()

    def test_reaction_fails_collection(self):
        step = db.ElementaryStep(db.ID())
        self.assertRaises(RuntimeError, lambda: step.has_reaction())
        self.assertRaises(RuntimeError, lambda: step.get_reaction())
        self.assertRaises(RuntimeError, lambda: step.set_reaction(db.ID()))
        self.assertRaises(RuntimeError, lambda: step.clear_reaction())

    def test_reaction_fails_id(self):
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep()
        step.link(coll)
        self.assertRaises(RuntimeError, lambda: step.has_reaction())
        self.assertRaises(RuntimeError, lambda: step.get_reaction())
        self.assertRaises(RuntimeError, lambda: step.set_reaction(db.ID()))
        self.assertRaises(RuntimeError, lambda: step.clear_reaction())

    def test_spline(self):
        # Basic setup
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep.make([db.ID()], [db.ID()], coll)
        assert step.has_id()

        # Checks
        knots = np.ndarray((3, 1), buffer=np.asarray(
            [0.0, 0.5, 1.0]
        ))
        data = np.ndarray((3, 4), buffer=np.asarray([
            [1.0, 2.0, 3.0, 4.0],
            [1.0, 2.0, 3.0, 4.0],
            [1.0, 2.0, 3.0, 4.0]
        ]))
        data = [
            [1.0, 2.0, 3.0, 4.0],
            [1.0, 2.0, 3.0, 4.0],
            [1.0, 2.0, 3.0, 4.0]
        ]
        elements = [utils.ElementType.H]
        ref = utils.bsplines.TrajectorySpline(elements, knots, data, 0.123)
        assert not step.has_spline()
        step.set_spline(ref)
        assert step.has_spline()
        spline = step.get_spline()
        assert spline.elements[0] == ref.elements[0]
        assert spline.ts_position == ref.ts_position
        for i in range(3):
            assert spline.knots[i] == ref.knots[i]
            for j in range(4):
                print(ref.data[i][j])
                print(spline.data[i][j])
                assert spline.data[i][j] == ref.data[i][j]
        step.clear_spline()
        assert not step.has_spline()

    def test_spline_fails_collection(self):
        step = db.ElementaryStep(db.ID())
        spline = utils.bsplines.TrajectorySpline([], [], [])
        self.assertRaises(RuntimeError, lambda: step.has_spline())
        self.assertRaises(RuntimeError, lambda: step.get_spline())
        self.assertRaises(RuntimeError, lambda: step.set_spline(spline))
        self.assertRaises(RuntimeError, lambda: step.clear_spline())

    def test_spline_fails_id(self):
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep()
        step.link(coll)
        spline = utils.bsplines.TrajectorySpline([], [], [])
        self.assertRaises(RuntimeError, lambda: step.has_spline())
        self.assertRaises(RuntimeError, lambda: step.get_spline())
        self.assertRaises(RuntimeError, lambda: step.set_spline(spline))
        self.assertRaises(RuntimeError, lambda: step.clear_spline())

    def test_path(self):
        # Basic setup
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep.make([db.ID()], [db.ID()], coll)
        assert step.has_id()

        # Checks  // Setup
        ref = [db.ID(), db.ID(), db.ID()]
        assert not step.has_path()
        step.set_path(ref)
        assert step.has_path()
        assert step.has_structure_in_path(ref[0])
        assert step.has_structure_in_path(ref[1])
        assert step.has_structure_in_path(ref[2])
        path = step.get_path()
        assert path[0] == ref[0]
        assert path[1] == ref[1]
        assert path[2] == ref[2]
        step.clear_path()
        assert not step.has_path()

    def test_path_fails_collection(self):
        step = db.ElementaryStep(db.ID())
        id = db.ID()
        self.assertRaises(RuntimeError, lambda: step.has_path())
        self.assertRaises(RuntimeError, lambda: step.get_path())
        self.assertRaises(RuntimeError, lambda: step.set_path([id]))
        self.assertRaises(RuntimeError, lambda: step.clear_path())
        self.assertRaises(RuntimeError, lambda: step.has_structure_in_path(id))
        self.assertRaises(RuntimeError, lambda: step.get_path(self.manager))

    def test_path_fails_id(self):
        coll = self.manager.get_collection("elementary_steps")
        step = db.ElementaryStep()
        step.link(coll)
        id = db.ID()
        self.assertRaises(RuntimeError, lambda: step.has_path())
        self.assertRaises(RuntimeError, lambda: step.get_path())
        self.assertRaises(RuntimeError, lambda: step.set_path([id]))
        self.assertRaises(RuntimeError, lambda: step.clear_path())
        self.assertRaises(RuntimeError, lambda: step.has_structure_in_path(id))
        self.assertRaises(RuntimeError, lambda: step.get_path(self.manager))
