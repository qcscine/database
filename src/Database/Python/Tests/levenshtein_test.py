# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

import scine_database as db
import unittest


class NumberPropertyTest(unittest.TestCase):

    def test_levenshtein_one(self):
        a = "kitten"
        b = "sitting"
        assert db.levenshtein_distance(a, b) == 3

    def test_levenshtein_two(self):
        assert db.levenshtein_distance("kitten", "sitting") == 3
        assert db.levenshtein_distance("a", "b") == 1
        assert db.levenshtein_distance("ab", "cd") == 2
        assert db.levenshtein_distance("ab", "cb") == 1

    def test_levenshtein_three(self):
        assert db.levenshtein_distance("kitten", "sitting") == 3
        assert db.levenshtein_distance("a", "b", 1, 1, 2) == 2
        assert db.levenshtein_distance("ab", "cde", 2, 1, 1) == 4
        assert db.levenshtein_distance("ab", "cde", 1, 2, 1) == 3
        assert db.levenshtein_distance("abe", "cb", 1, 2, 1) == 3
        assert db.levenshtein_distance("abe", "cb", 2, 1, 1) == 2
