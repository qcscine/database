# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.
See LICENSE.txt for details.
"""

import scine_database as db
import unittest
import numpy as np
from scipy.sparse import rand

import os


class SparseMatrixPropertyTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP', '127.0.0.1')
        self.manager.credentials.port = int(os.environ.get('TEST_MONGO_DB_PORT', 27017))
        self.manager.credentials.database_name = "unittest_db_SparseMatrixPropertyTest"
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
        ref = rand(1000, 10, density=0.2, format='csr')
        test = db.SparseMatrixProperty.make("density_matrix", model, ref, s, c, coll)
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
        assert np.array_equal(data_db.todense(), ref.todense())

    def test_make_two(self):
        # Setup
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        ref = rand(1000, 10, density=0.2, format='csr')
        test = db.SparseMatrixProperty.make("density_matrix", model, ref, coll)
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
        assert np.array_equal(data_db.todense(), ref.todense())

    def test_create_one(self):
        # Setup
        coll = self.manager.get_collection("properties")
        c = db.ID()
        s = db.ID()
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.SparseMatrixProperty()
        test.link(coll)
        ref = rand(1000, 10, density=0.2, format='csr')
        test.create(model, "density_matrix", s, c, ref)
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
        assert np.array_equal(data_db.todense(), ref.todense())

    def test_create_two(self):
        # Setup
        coll = self.manager.get_collection("properties")
        model = db.Model("dft", "pbe", "def2-svp")
        test = db.SparseMatrixProperty()
        test.link(coll)
        ref = rand(1000, 10, density=0.2, format='csr')
        test.create(model, "density_matrix", ref)
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
        assert np.array_equal(data_db.todense(), ref.todense())

    def test_data(self):
        # Setup
        coll = self.manager.get_collection("properties")
        s = db.ID()
        c = db.ID()
        model = db.Model("dft", "pbe", "def2-svp")
        ref = rand(1000, 10, density=0.2, format='csr')
        test = db.SparseMatrixProperty.make("density_matrix", model, ref, s, c, coll)
        assert test.has_id()

        assert np.array_equal(test.get_data().todense(), ref.todense())
        ref2 = rand(1000, 10, density=0.2, format='csr')
        test.set_data(ref2)
        assert np.array_equal(test.get_data().todense(), ref2.todense())

    def test_data_failure(self):
        # Setup
        coll = self.manager.get_collection("properties")
        test = db.SparseMatrixProperty()
        ref = rand(1000, 10, density=0.2, format='csr')
        self.assertRaises(RuntimeError, lambda: test.set_data(ref))
        self.assertRaises(RuntimeError, lambda: test.get_data())
        test.link(coll)
        self.assertRaises(RuntimeError, lambda: test.set_data(ref))
        self.assertRaises(RuntimeError, lambda: test.get_data())
