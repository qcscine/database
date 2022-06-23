# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

import scine_utilities as utils
import scine_database as db
import unittest
import os


class CalculationTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP') or '127.0.0.1'
        self.manager.credentials.database_name = "unittest_db_CalculationTest"
        self.manager.connect()
        self.manager.init()

    def tearDown(self):
        self.manager.wipe()

    def test_creation(self):
        # Setup
        s1 = db.ID()
        s2 = db.ID()
        s3 = db.ID()
        coll = self.manager.get_collection("calculations")
        j = db.Job("geo_opt")
        m = db.Model("dft", "pbe", "def2-svp")
        calc = db.Calculation.make(m, j, [s1, s2, s3], coll)
        assert calc.has_id()

        # Check Fields
        status = calc.get_status()
        auxiliaries = calc.get_auxiliaries()
        structures = calc.get_structures()
        settings = calc.get_settings()
        model = calc.get_model()
        results = calc.get_results()
        job = calc.get_job()
        output = calc.get_raw_output()
        comment = calc.get_comment()

        assert status == db.Status.CONSTRUCTION
        assert len(auxiliaries) == 0
        assert len(structures) == 3
        assert len(settings) == 0
        assert len(results.structure_ids) == 0
        assert len(results.property_ids) == 0
        assert len(results.elementary_step_ids) == 0
        assert model.method == "pbe"
        assert model.basis_set == "def2-svp"
        assert model.spin_mode == "any"
        assert job.order == "geo_opt"
        assert output == ""
        assert comment == ""

    def test_priority(self):
        # Setup
        s = [db.ID(), db.ID(), db.ID()]
        coll = self.manager.get_collection("calculations")
        j = db.Job("geo_opt")
        m = db.Model("dft", "pbe", "def2-svp")
        calc = db.Calculation.make(m, j, s, coll)
        assert calc.has_id()

        # Priority Functionalities
        priority = calc.get_priority()
        assert priority == 10
        calc.set_priority(1)
        priority = calc.get_priority()
        assert priority == 1

    def test_property_fails_collection(self):
        calc = db.Calculation()
        self.assertRaises(RuntimeError, lambda: calc.set_priority(1))
        self.assertRaises(RuntimeError, lambda: calc.get_priority())

    def test_property_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        self.assertRaises(RuntimeError, lambda: calc.set_priority(1))
        self.assertRaises(RuntimeError, lambda: calc.get_priority())

    def test_property_fails_range(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation(db.ID())
        calc.link(coll)
        self.assertRaises(ValueError, lambda: calc.set_priority(0))
        self.assertRaises(ValueError, lambda: calc.set_priority(123))

    def test_status(self):
        # Setup
        s = [db.ID(), db.ID(), db.ID()]
        coll = self.manager.get_collection("calculations")
        j = db.Job("geo_opt")
        m = db.Model("dft", "pbe", "def2-svp")
        calc = db.Calculation.make(m, j, s, coll)
        assert calc.has_id()

        # Functionalities
        status = calc.get_status()
        assert status == db.Status.CONSTRUCTION
        assert status != db.Status.NEW
        calc.set_status(db.Status.FAILED)
        status = calc.get_status()
        assert status == db.Status.FAILED

    def test_status_fails_collection(self):
        calc = db.Calculation()
        self.assertRaises(RuntimeError, lambda: calc.set_status(db.Status.NEW))
        self.assertRaises(RuntimeError, lambda: calc.get_status())

    def test_status_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        self.assertRaises(RuntimeError, lambda: calc.set_status(db.Status.NEW))
        self.assertRaises(RuntimeError, lambda: calc.get_status())

    def test_job(self):
        # Setup
        s = [db.ID(), db.ID(), db.ID()]
        coll = self.manager.get_collection("calculations")
        j = db.Job("geo_opt")
        m = db.Model("dft", "pbe", "def2-svp")
        calc = db.Calculation.make(m, j, s, coll)
        assert calc.has_id()

        # Job Functionalities
        job = calc.get_job()
        assert job.order == "geo_opt"
        assert job.memory == 1.0
        assert job.cores == 1
        assert job.disk == 1
        job.order = "asdf"
        job.memory = 123.456
        job.cores = 15
        job.disk = 654.123
        calc.set_job(job)
        job_db = calc.get_job()
        assert job.order == job_db.order
        assert job.memory == job_db.memory
        assert job.cores == job_db.cores
        assert job.disk == job_db.disk

    def test_job_fails_collection(self):
        calc = db.Calculation()
        job = db.Job("geo_opt")
        self.assertRaises(RuntimeError, lambda: calc.get_job())
        self.assertRaises(RuntimeError, lambda: calc.set_job(job))

    def test_job_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        job = db.Job("geo_opt")
        self.assertRaises(RuntimeError, lambda: calc.get_job())
        self.assertRaises(RuntimeError, lambda: calc.set_job(job))

    def test_model(self):
        # Setup
        s = [db.ID(), db.ID(), db.ID()]
        coll = self.manager.get_collection("calculations")
        j = db.Job("geo_opt")
        m = db.Model("dft", "pbe", "def2-svp")
        calc = db.Calculation.make(m, j, s, coll)
        assert calc.has_id()

        # Model Functionalities
        model = calc.get_model()
        assert model.method == "pbe"
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
        calc.set_model(model)
        model_db = calc.get_model()
        assert model.spin_mode == model_db.spin_mode
        assert model.basis_set == model_db.basis_set
        assert model.program == model_db.program
        assert model.version == model_db.version
        assert model.solvation == model_db.solvation
        assert model.solvent == model_db.solvent
        assert model.embedding == model_db.embedding
        assert model.periodic_boundaries == model_db.periodic_boundaries
        assert model.external_field == model_db.external_field

    def test_model_fails_collection(self):
        calc = db.Calculation()
        model = db.Model("dft", "pbe", "def2-svp")
        self.assertRaises(RuntimeError, lambda: calc.get_model())
        self.assertRaises(RuntimeError, lambda: calc.set_model(model))

    def test_model_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        model = db.Model("dft", "pbe", "def2-svp")
        self.assertRaises(RuntimeError, lambda: calc.get_model())
        self.assertRaises(RuntimeError, lambda: calc.set_model(model))

    def test_settings(self):
        # Setup
        s = [db.ID(), db.ID(), db.ID()]
        coll = self.manager.get_collection("calculations")
        j = db.Job("geo_opt")
        m = db.Model("dft", "pbe", "def2-svp")
        calc = db.Calculation.make(m, j, s, coll)
        assert calc.has_id()

        # Setting Functionalities
        settings = calc.get_settings()
        assert len(settings) == 0

        calc.set_setting("foo", "bar")
        calc.set_setting("spam", 4)
        settings_db = calc.get_settings()
        assert settings_db["foo"] == "bar"
        assert settings_db["spam"] == 4
        assert str(calc.get_setting("foo")) == "bar"
        assert int(calc.get_setting("spam")) == 4

        calc.remove_setting("foo")
        settings_db = calc.get_settings()
        assert int(settings_db["spam"]) == 4
        assert calc.get_setting("spam") == 4
        assert calc.has_setting("spam")

        calc.clear_settings()
        assert len(settings) == 0

        fruit_bowl = utils.ValueCollection({
            "pear": 2,
            "fig": 5
        })
        climbing_plants = utils.ValueCollection({
            "annoying": True,
            "difficult_removal": True,
            "pretty": True,
            "species": 401293,
        })
        plants = utils.ValueCollection({
            "climbing": climbing_plants,
        })
        room_decoration = utils.ValueCollection({
            "plants": plants,
        })

        cars = utils.ValueCollection({
            "classic": ["triumph", "pontiac", "plymouth"]
        })

        apple_list = ["Hello", "I", "am", "an", "apple"]
        banana_list = [1, 2, 3, 4]
        orange_list = [5.0, 6.0, 7.0, 8.0]
        cherry_list = [fruit_bowl, room_decoration, cars]
        settings = utils.ValueCollection({
            "foo": "bar",
            "spam": 4,
            "apple": apple_list,
            "banana": banana_list,
            "orange": orange_list,
            "cherry": cherry_list,
        })

        calc.set_settings(settings)
        assert calc.get_setting("foo") == "bar"
        assert calc.get_setting("spam") == 4
        assert calc.get_setting("apple") == apple_list
        assert calc.get_setting("banana") == banana_list
        assert calc.get_setting("orange") == orange_list
        assert calc.get_setting("cherry") == cherry_list

    def test_settings_fail_collection(self):
        calc = db.Calculation(db.ID())
        self.assertRaises(RuntimeError, lambda: calc.get_setting("foo"))
        self.assertRaises(RuntimeError, lambda: calc.has_setting("foo"))
        self.assertRaises(RuntimeError, lambda: calc.remove_setting("foo"))
        self.assertRaises(RuntimeError, lambda: calc.set_setting("foo", 7))
        self.assertRaises(RuntimeError, lambda: calc.set_settings(
            utils.ValueCollection({})))
        self.assertRaises(RuntimeError, lambda: calc.get_settings())
        self.assertRaises(RuntimeError, lambda: calc.clear_settings())

    def test_settings_fail_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        self.assertRaises(RuntimeError, lambda: calc.get_setting("foo"))
        self.assertRaises(RuntimeError, lambda: calc.has_setting("foo"))
        self.assertRaises(RuntimeError, lambda: calc.remove_setting("foo"))
        self.assertRaises(RuntimeError, lambda: calc.set_setting("foo", 7))
        self.assertRaises(RuntimeError, lambda: calc.set_settings(
            utils.ValueCollection({})))
        self.assertRaises(RuntimeError, lambda: calc.get_settings())
        self.assertRaises(RuntimeError, lambda: calc.clear_settings())

    def test_results(self):
        s1 = db.ID()
        s2 = db.ID()
        s3 = db.ID()
        coll = self.manager.get_collection("calculations")
        model = db.Model("dft", "pbe", "def2-svp")
        job = db.Job("geo_opt")
        calc = db.Calculation.make(model, job, [s1, s2, s3], coll)
        job = db.Job("sp")
        calc2 = db.Calculation.make(model, job, [s1, s2, s3], coll)
        assert calc.has_id()
        assert calc2.has_id()

        # Result Functionalities
        id1 = db.ID()
        id2 = db.ID()
        id3 = db.ID()
        id4 = db.ID()
        id5 = db.ID()
        id6 = db.ID()
        id7 = db.ID()
        id8 = db.ID()
        id9 = db.ID()

        results = calc.get_results()
        assert len(results.structure_ids) == 0
        assert len(results.property_ids) == 0
        assert len(results.elementary_step_ids) == 0
        results2 = calc2.get_results()
        assert len(results2.structure_ids) == 0
        assert len(results2.property_ids) == 0
        assert len(results2.elementary_step_ids) == 0

        results.add_structure(id1)
        results.add_structure(id2)
        results.add_property(id3)
        results.add_property(id4)
        results.add_elementary_step(id5)
        results.add_elementary_step(id6)
        calc.set_results(results)
        results_db = calc.get_results()
        assert len(results_db.structure_ids) == 2
        assert len(results_db.property_ids) == 2
        assert len(results_db.elementary_step_ids) == 2
        assert results_db.structure_ids[0] == id1
        assert results_db.structure_ids[1] == id2
        assert results_db.property_ids[0] == id3
        assert results_db.property_ids[1] == id4
        assert results_db.elementary_step_ids[0] == id5
        assert results_db.elementary_step_ids[1] == id6

        results2.structure_ids = [id1, id7]
        results2.property_ids = [id3, id8]
        results2.elementary_step_ids = [id5, id9]
        calc2.set_results(results2)
        results_db2 = calc2.get_results()
        assert len(results_db2.structure_ids) == 2
        assert len(results_db2.property_ids) == 2
        assert len(results_db2.elementary_step_ids) == 2
        assert results_db2.structure_ids[0] == id1
        assert results_db2.structure_ids[1] == id7
        assert results_db2.property_ids[0] == id3
        assert results_db2.property_ids[1] == id8
        assert results_db2.elementary_step_ids[0] == id5
        assert results_db2.elementary_step_ids[1] == id9

        combinedResults = results_db + results_db2
        assert len(combinedResults.structure_ids) == 3
        assert len(combinedResults.property_ids) == 3
        assert len(combinedResults.elementary_step_ids) == 3
        assert combinedResults.structure_ids[0] == id1
        assert combinedResults.structure_ids[1] == id2
        assert combinedResults.structure_ids[2] == id7
        assert combinedResults.property_ids[0] == id3
        assert combinedResults.property_ids[1] == id4
        assert combinedResults.property_ids[2] == id8
        assert combinedResults.elementary_step_ids[0] == id5
        assert combinedResults.elementary_step_ids[1] == id6
        assert combinedResults.elementary_step_ids[2] == id9

        calc.clear_results()
        results_db = calc.get_results()
        assert len(results_db.structure_ids) == 0
        assert len(results_db.property_ids) == 0
        assert len(results_db.elementary_step_ids) == 0

    def test_results_fails_collection(self):
        calc = db.Calculation()
        self.assertRaises(RuntimeError, lambda: calc.set_results(db.Results()))
        self.assertRaises(RuntimeError, lambda: calc.get_results())
        self.assertRaises(RuntimeError, lambda: calc.clear_results())

    def test_results_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        self.assertRaises(RuntimeError, lambda: calc.set_results(db.Results()))
        self.assertRaises(RuntimeError, lambda: calc.get_results())
        self.assertRaises(RuntimeError, lambda: calc.clear_results())

    def test_structures(self):
        s1 = db.ID()
        s2 = db.ID()
        s3 = db.ID()
        coll = self.manager.get_collection("calculations")
        model = db.Model("dft", "pbe", "def2-svp")
        job = db.Job("geo_opt")
        calc = db.Calculation.make(model, job, [s1, s2, s3], coll)
        assert calc.has_id()

        # Structure Functionalities
        s4 = db.ID()
        s5 = db.ID()
        s6 = db.ID()

        structures = calc.get_structures()
        assert len(structures) == 3
        assert structures[0] == s1
        assert structures[1] == s2
        assert structures[2] == s3

        calc.add_structure(s4)
        calc.remove_structure(s1)
        structures = calc.get_structures()
        assert len(structures) == 3
        assert structures[0] == s2
        assert structures[1] == s3
        assert structures[2] == s4

        calc.set_structures([s4, s5, s6])
        structures = calc.get_structures()
        assert len(structures) == 3
        assert structures[0] == s4
        assert structures[1] == s5
        assert structures[2] == s6
        assert calc.has_structure(s4)

        calc.clear_structures()
        structures = calc.get_structures()
        assert len(structures) == 0
        assert not calc.has_structure(s4)

    def test_structures_fails_collection(self):
        calc = db.Calculation()
        self.assertRaises(RuntimeError, lambda: calc.add_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: calc.has_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: calc.remove_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: calc.set_structures([]))
        self.assertRaises(RuntimeError, lambda: calc.get_structures())
        self.assertRaises(RuntimeError, lambda: calc.clear_structures())

    def test_structures_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        self.assertRaises(RuntimeError, lambda: calc.add_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: calc.has_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: calc.remove_structure(db.ID()))
        self.assertRaises(RuntimeError, lambda: calc.set_structures([]))
        self.assertRaises(RuntimeError, lambda: calc.get_structures())
        self.assertRaises(RuntimeError, lambda: calc.clear_structures())

    def test_auxiliaries(self):
        s1 = db.ID()
        s2 = db.ID()
        s3 = db.ID()
        coll = self.manager.get_collection("calculations")
        model = db.Model("dft", "pbe", "def2-svp")
        job = db.Job("geo_opt")
        calc = db.Calculation.make(model, job, [s1, s2, s3], coll)
        assert calc.has_id()

        # Auxiliaries Functionalities
        id1 = db.ID()
        id2 = db.ID()
        id3 = db.ID()
        id4 = db.ID()
        id5 = db.ID()
        calc.set_auxiliary("foo", id1)
        calc.set_auxiliary("bar", id2)
        calc.set_auxiliary("foobar", id3)
        calc.set_auxiliary("foobar", id2)
        assert calc.has_auxiliary("foo")
        aux = {
            "foo": id3,
            "bar": id4,
            "barfoo": id5
        }
        calc.remove_auxiliary("foo")
        assert not calc.has_auxiliary("foo")
        calc.set_auxiliaries(aux)
        assert calc.has_auxiliary("foo")
        assert calc.get_auxiliary("foo") == id3
        assert not calc.has_auxiliary("foobar")
        calc.clear_auxiliaries()
        assert not calc.has_auxiliary("foo")
        assert not calc.has_auxiliary("bar")
        assert not calc.has_auxiliary("foobar")
        assert not calc.has_auxiliary("barfoo")

    def test_auxiliaries_fails_collection(self):
        calc = db.Calculation()
        self.assertRaises(
            RuntimeError, lambda: calc.set_auxiliary("foo", db.ID()))
        self.assertRaises(RuntimeError, lambda: calc.get_auxiliary("foo"))
        self.assertRaises(RuntimeError, lambda: calc.has_auxiliary("foo"))
        self.assertRaises(RuntimeError, lambda: calc.remove_auxiliary("foo"))
        self.assertRaises(RuntimeError, lambda: calc.set_auxiliaries({}))
        self.assertRaises(RuntimeError, lambda: calc.get_auxiliaries())
        self.assertRaises(RuntimeError, lambda: calc.clear_auxiliaries())

    def test_auxiliaries_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        self.assertRaises(
            RuntimeError, lambda: calc.set_auxiliary("foo", db.ID()))
        self.assertRaises(RuntimeError, lambda: calc.get_auxiliary("foo"))
        self.assertRaises(RuntimeError, lambda: calc.has_auxiliary("foo"))
        self.assertRaises(RuntimeError, lambda: calc.remove_auxiliary("foo"))
        self.assertRaises(RuntimeError, lambda: calc.set_auxiliaries({}))
        self.assertRaises(RuntimeError, lambda: calc.get_auxiliaries())
        self.assertRaises(RuntimeError, lambda: calc.clear_auxiliaries())

    def test_raw_output(self):
        s1 = db.ID()
        s2 = db.ID()
        s3 = db.ID()
        coll = self.manager.get_collection("calculations")
        model = db.Model("dft", "pbe", "def2-svp")
        job = db.Job("geo_opt")
        calc = db.Calculation.make(model, job, [s1, s2, s3], coll)
        assert calc.has_id()

        # Runtime Functionalities
        assert not calc.has_raw_output()
        assert calc.get_raw_output() == ""

        calc.set_raw_output("dummy")
        assert calc.has_raw_output()
        assert calc.get_raw_output() == "dummy"

        calc.clear_raw_output()
        assert not calc.has_raw_output()
        assert calc.get_raw_output() == ""

    def test_raw_output_fails_collection(self):
        calc = db.Calculation()
        self.assertRaises(RuntimeError, lambda: calc.set_raw_output(""))
        self.assertRaises(RuntimeError, lambda: calc.get_raw_output())
        self.assertRaises(RuntimeError, lambda: calc.has_raw_output())
        self.assertRaises(RuntimeError, lambda: calc.clear_raw_output())

    def test_raw_output_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        self.assertRaises(RuntimeError, lambda: calc.set_raw_output(""))
        self.assertRaises(RuntimeError, lambda: calc.get_raw_output())
        self.assertRaises(RuntimeError, lambda: calc.has_raw_output())
        self.assertRaises(RuntimeError, lambda: calc.clear_raw_output())

    def test_comment(self):
        s1 = db.ID()
        s2 = db.ID()
        s3 = db.ID()
        coll = self.manager.get_collection("calculations")
        model = db.Model("dft", "pbe", "def2-svp")
        job = db.Job("geo_opt")
        calc = db.Calculation.make(model, job, [s1, s2, s3], coll)
        assert calc.has_id()

        # Runtime Functionalities
        assert not calc.has_comment()
        assert calc.get_comment() == ""

        calc.set_comment("dummy")
        assert calc.has_comment()
        assert calc.get_comment() == "dummy"

        calc.clear_comment()
        assert not calc.has_comment()
        assert calc.get_comment() == ""

    def test_comment_fails_collection(self):
        calc = db.Calculation()
        self.assertRaises(RuntimeError, lambda: calc.set_comment(""))
        self.assertRaises(RuntimeError, lambda: calc.get_comment())
        self.assertRaises(RuntimeError, lambda: calc.has_comment())
        self.assertRaises(RuntimeError, lambda: calc.clear_comment())

    def test_comment_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        self.assertRaises(RuntimeError, lambda: calc.set_comment(""))
        self.assertRaises(RuntimeError, lambda: calc.get_comment())
        self.assertRaises(RuntimeError, lambda: calc.has_comment())
        self.assertRaises(RuntimeError, lambda: calc.clear_comment())

    def test_executor(self):
        s1 = db.ID()
        s2 = db.ID()
        s3 = db.ID()
        coll = self.manager.get_collection("calculations")
        model = db.Model("dft", "pbe", "def2-svp")
        job = db.Job("geo_opt")
        calc = db.Calculation.make(model, job, [s1, s2, s3], coll)
        assert calc.has_id()

        # Runtime Functionalities
        assert not calc.has_executor()
        assert calc.get_executor() == ""

        calc.set_executor("dummy")
        assert calc.has_executor()
        assert calc.get_executor() == "dummy"

        calc.clear_executor()
        assert not calc.has_executor()
        assert calc.get_executor() == ""

    def test_executor_fails_collection(self):
        calc = db.Calculation()
        self.assertRaises(RuntimeError, lambda: calc.set_executor(""))
        self.assertRaises(RuntimeError, lambda: calc.get_executor())
        self.assertRaises(RuntimeError, lambda: calc.has_executor())
        self.assertRaises(RuntimeError, lambda: calc.clear_executor())

    def test_executor_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        self.assertRaises(RuntimeError, lambda: calc.set_executor(""))
        self.assertRaises(RuntimeError, lambda: calc.get_executor())
        self.assertRaises(RuntimeError, lambda: calc.has_executor())
        self.assertRaises(RuntimeError, lambda: calc.clear_executor())

    def test_runtime(self):
        s1 = db.ID()
        s2 = db.ID()
        s3 = db.ID()
        coll = self.manager.get_collection("calculations")
        model = db.Model("dft", "pbe", "def2-svp")
        job = db.Job("geo_opt")
        calc = db.Calculation.make(model, job, [s1, s2, s3], coll)
        assert calc.has_id()

        # Runtime Functionalities
        assert not calc.has_runtime()
        self.assertRaises(RuntimeError, lambda: calc.get_runtime())

        calc.set_runtime(125.0125)
        assert calc.has_runtime()
        runtime = calc.get_runtime()
        assert runtime == 125.0125

        calc.clear_runtime()
        assert not calc.has_runtime()
        self.assertRaises(RuntimeError, lambda: calc.get_runtime())

    def test_runtime_fails_collection(self):
        calc = db.Calculation()
        self.assertRaises(RuntimeError, lambda: calc.set_runtime(1.2))
        self.assertRaises(RuntimeError, lambda: calc.get_runtime())
        self.assertRaises(RuntimeError, lambda: calc.has_runtime())
        self.assertRaises(RuntimeError, lambda: calc.clear_runtime())

    def test_runtime_fails_id(self):
        coll = self.manager.get_collection("calculations")
        calc = db.Calculation()
        calc.link(coll)
        self.assertRaises(RuntimeError, lambda: calc.set_runtime(1.2))
        self.assertRaises(RuntimeError, lambda: calc.get_runtime())
        self.assertRaises(RuntimeError, lambda: calc.has_runtime())
        self.assertRaises(RuntimeError, lambda: calc.clear_runtime())
