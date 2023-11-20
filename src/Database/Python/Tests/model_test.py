# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.
See LICENSE.txt for details.
"""

import scine_utilities as utils
import scine_database as db
import unittest
import os
import pickle


class ModelTest(unittest.TestCase):

    def test_model_completion(self):
        settings = utils.Settings("test", {
            "temperature": 1.0,
            "electronic_temperature": 2.0,
            "method": "3",
            "method_family": "4",
            "spin_mode": "5",
            "program": "6",
            "version": "7",
            "basis_set": "8",
            "solvation": "9",
            "solvent": "10",
            "embedding": "11",
            "periodic_boundaries": "12",
            "external_field": "none",
            "pressure": 100000.0
        })

        model = db.Model("should-not-change", "any", "any")
        model.program = "should-not-change"
        model.version = "should-not-change"
        model.spin_mode = "ANY"
        model.temperature = "any"
        model.electronic_temperature = "any"
        model.solvation = "any"
        model.solvent = "any"
        model.embedding = "any"
        model.periodic_boundaries = "any"
        model.external_field = ""
        model.pressure = "100000.0"
        model.complete_model(settings)

        assert float(model.temperature) == 1.0
        assert float(model.electronic_temperature) == 2.0
        assert model.method == "3"
        assert model.method_family == "should-not-change"
        assert model.spin_mode == "5"
        assert model.version == "should-not-change"
        assert model.program == "should-not-change"
        assert model.basis_set == "8"
        assert model.solvation == "9"
        assert model.solvent == "10"
        assert model.embedding == "11"
        assert model.periodic_boundaries == "12"
        assert model.external_field == ""
        assert abs(float(model.pressure) - 100000.0) < 1e-9

        # check if we throw because of collision
        model.embedding = "something"
        self.assertRaises(RuntimeError, lambda: model.complete_model(settings))

    def test_settings_completion(self):
        settings = utils.Settings("test", {
            "temperature": 298.15,
            "electronic_temperature": 300.0,
            "method": "",
            "method_family": "should-not-change",
            "spin_mode": "ANY",
            "program": "should-not-change",
            "version": "should-not-change",
            "basis_set": "",
            "solvation": "",
            "solvent": "",
            "embedding": "any",
            "periodic_boundaries": "",
            "external_field": "",
            "pressure": 100000.0,
        })
        model = db.Model("should-not-change", "any", "def2-tzvp")
        model.program = "should-not-change"
        model.version = "should-not-change"
        model.spin_mode = "unrestricted"
        model.temperature = "any"
        model.electronic_temperature = 0.0
        model.solvation = "pcm"
        model.solvent = "water"
        model.embedding = "fde"
        model.periodic_boundaries = "none"
        model.external_field = ""
        model.pressure = "100000.0"

        model.complete_settings(settings)

        assert settings["temperature"] == 298.15
        assert settings["electronic_temperature"] == 0.0
        assert settings["method"] == "any"
        assert settings["method_family"] == "should-not-change"
        assert settings["spin_mode"] == "unrestricted"
        assert settings["version"] == "should-not-change"
        assert settings["program"] == "should-not-change"
        assert settings["basis_set"] == "def2-tzvp"
        assert settings["solvation"] == "pcm"
        assert settings["solvent"] == "water"
        assert settings["embedding"] == "fde"
        assert settings["periodic_boundaries"] == "none"
        assert settings["external_field"] == ""

        # check if we throw for 'none' temperature
        model.temperature = "none"
        self.assertRaises(RuntimeError, lambda: model.complete_settings(settings))

    def test_equality_works(self):
        lhs = db.Model("dft", "any", "none", "none")
        rhs = db.Model("dft", "something", "", "none")
        assert lhs == rhs
        lhs.program = "sparrow"
        rhs.program = "sparrow"
        assert lhs == rhs
        rhs.program = "something_different"
        assert lhs != rhs
        lhs.program = "any"
        rhs.program = "something"
        assert lhs == rhs
        rhs.program = "none"
        assert lhs != rhs
        rhs.program = ""
        assert lhs != rhs

    def test_model_output_string(self):
        m = db.Model("any", "any", "any", "any")
        names = [
            "temperature",
            "electronic_temperature",
            "method",
            "method_family",
            "spin_mode",
            "program",
            "version",
            "basis_set",
            "solvation",
            "solvent",
            "embedding",
            "periodic_boundaries",
            "external_field",
            "pressure"
        ]
        m.temperature = 0.0
        m.electronic_temperature = "1.0"
        m.method = "2"
        m.method_family = "3"
        m.spin_mode = "4"
        m.program = "5"
        m.version = "6"
        m.basis_set = "7"
        m.solvation = "8"
        m.solvent = "9"
        m.embedding = "10"
        m.periodic_boundaries = "11"
        m.external_field = "12"
        m.pressure = "13.0"
        s = str(m)
        for name in names:
            for line in s.splitlines():
                target = name + " :"
                if target in line:
                    break
            assert target in line

    def test_model_pickle(self):
        model = db.Model("dft", "something", "", "none")
        with open("test_model_pickle.pkl", "wb") as f:
            pickle.dump(model, f)
        with open("test_model_pickle.pkl", "rb") as f:
            loaded_model = pickle.load(f)
        assert loaded_model == model
        os.remove("test_model_pickle.pkl")
