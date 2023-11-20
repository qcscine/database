# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.
See LICENSE.txt for details.
"""

import scine_database as db
import unittest


class IdTest(unittest.TestCase):

    def test_string_construction(self):
        test = db.ID("5be431a11afe220ada32c1d4")
        assert "5be431a11afe220ada32c1d4" == test.string()

    def test_comparison(self):
        test1 = db.ID("5be431a11afe220ada32c1d4")
        test2 = db.ID("5be431a11afe220ada32c1d5")
        assert test1 == test1
        assert test1 != test2

    def test_uniqueness(self):
        ref = db.ID()
        for _ in range(20):
            assert db.ID() != ref

    def test_copy(self):
        ref = db.ID()
        copy = ref
        assert copy == ref
