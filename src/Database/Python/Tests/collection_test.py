# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.
See LICENSE.txt for details.
"""

import scine_database as db
import unittest
import os
from json import dumps


class CollectionTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP', '127.0.0.1')
        self.manager.credentials.port = int(os.environ.get('TEST_MONGO_DB_PORT', 27017))
        self.manager.credentials.database_name = "unittest_db_CollectionTest"
        self.manager.connect()
        self.manager.init()

    def tearDown(self):
        self.manager.wipe()

    def test_query_id(self):
        coll = self.manager.get_collection("compounds")
        cid = db.ID()
        comp = db.Compound.make([cid], coll)
        # Check
        assert coll.has(comp.get_id())

    def test_return_by_id(self):
        coll = self.manager.get_collection("compounds")
        cid = db.ID()
        comp = db.Compound.make([cid], coll)
        # Check
        result = coll.get_compound(comp.get_id())
        assert comp.get_id() == result.get_id()

    def test_return_by_id_fails(self):
        coll = self.manager.get_collection("compounds")
        cid = db.ID()
        self.assertRaises(RuntimeError, lambda: coll.get_compound(cid))

    def test_query_by_dict(self):
        # Make sure the DB is clean in order to allow for accurate counts.
        self.manager.wipe()
        self.manager.init()
        # Setup
        coll = self.manager.get_collection("compounds")
        id1 = db.ID()
        id2 = db.ID()
        id3 = db.ID()
        comp1 = db.Compound.make([id1], coll)
        comp2 = db.Compound.make([id1], coll)
        _ = db.Compound.make([id2], coll)
        _ = db.Compound.make([id3], coll)

        query = {"structures": {"$eq": {"$oid": id1.string()}}}
        vec1 = coll.query_compounds(dumps(query))
        vec2 = coll.query_structures(dumps(query))

        # Check
        assert len(vec1) == 2
        assert vec1[0].get_id() == comp1.get_id()
        assert vec1[1].get_id() == comp2.get_id()
        assert len(vec2) == 0

    def test_get_one(self):
        # Make sure the DB is clean in order to allow for accurate counts.
        self.manager.wipe()
        self.manager.init()
        # Setup
        coll = self.manager.get_collection("compounds")
        id1 = db.ID()
        _ = db.ID()
        id3 = db.ID()
        comp1 = db.Compound.make([id1], coll)
        _ = db.Compound.make([id1], coll)
        _ = db.Compound.make([id3], coll)
        # Create a compound that could be found but should not to catch false
        #   positives
        query1 = {"structures": {"$eq": {"$oid": id1.string()}}}
        query2 = {"structures": {"$eq": {"$oid": id3.string()}}}
        r1 = coll.get_one_compound(dumps(query1))
        r2 = coll.get_one_compound(dumps(query2))

        # Check
        assert r1.has_id()
        assert r1.id() == comp1.id()
        assert r2.has_id()
        self.assertRaises(RuntimeError, lambda: coll.get_one_structure(dumps(query1)))

    def get_one_with_sort(self):
        self.manager.wipe()
        self.manager.init()

        # Setup
        coll = self.manager.get_collection("compounds")
        id1 = db.ID()
        id2 = db.ID()

        comp1 = db.Compound.make([id1], coll)
        comp2 = db.Compound.make([id2, id1], coll)

        query = {"structures": {"$all": [{"$oid": id1.string()}]}}
        sort1 = {"_id": 1}
        sort2 = {"_id": -1}
        r1 = coll.get_one_compound(query)
        r2 = coll.get_one_compound(query, sort1)
        r3 = coll.get_one_compound(query, sort2)

        assert r1.has_id()
        assert r2.has_id()
        assert r3.has_id()

        assert r1.id() == comp1.id()
        assert r2.id() == comp1.id()
        assert r3.id() == comp2.id()

        assert r1.get_structures()[0] == id1
        assert r2.get_structures()[0] == id1
        assert r3.get_structures()[0] == id2
        self.assertRaises(RuntimeError, lambda: coll.get_one_structure(dumps(query)))

    def get_one_and_modify(self):
        self.manager.wipe()
        self.manager.init()

        coll = self.manager.get_collection("compounds")
        id1 = db.ID()
        id2 = db.ID()
        id3 = db.ID()
        id4 = db.ID()
        comp1 = db.Compound.make([id1], coll)
        _ = db.Compound.make([id1], coll)
        _ = db.Compound.make([id3], coll)

        query1 = {"structures": {"$eq": {"$oid": id1.string()}}}
        query2 = {"structures": {"$eq": {"$oid": id2.string()}}}
        update = {"$set": {"reactions": [{"$oid": id4.string()}]}}
        r1 = coll.get_and_update_one_compound(query1, update)
        r2 = coll.get_and_update_one_compound(query2, update)

        assert r1.has_id()
        assert r1.id() == comp1.id()
        assert r1.get_reactions()[0] == id4
        assert not r2.has_id()
        self.assertRaises(RuntimeError, lambda: coll.get_and_update_one_structure(dumps(query1, update)))

    def get_one_and_modify_with_sort(self):
        self.manager.wipe()
        self.manager.init()

        coll = self.manager.get_collection("compounds")
        id1 = db.ID()
        id2 = db.ID()
        id3 = db.ID()
        _ = db.Compound.make([id1], coll)
        comp2 = db.Compound.make([id2, id1], coll)

        query = {"structures": {"$all": [{"$oid": id1.string()}]}}
        update = {"$set": {"reactions": [{"$oid": id3.string()}]}}
        sort = {"_id": -1}
        r1 = coll.get_and_update_one_compound(query, update, sort)
        assert r1.has_id()
        assert r1.id() == comp2.id()
        assert r1.get_reactions()[0] == id3
        self.assertRaises(RuntimeError, lambda: coll.get_one_structure(dumps(query, update, sort)))

    def test_random_select(self):
        # Make sure the DB is clean in order to allow for accurate counts.
        self.manager.wipe()
        self.manager.init()
        # Setup
        coll = self.manager.get_collection("compounds")
        id1 = db.ID()
        id2 = db.ID()
        id3 = db.ID()
        _ = db.Compound.make([id1], coll)
        _ = db.Compound.make([id2], coll)
        _ = db.Compound.make([id3], coll)

        # Check correct number
        nSamples = 2
        vec1 = coll.random_select_compounds(nSamples)
        assert len(vec1) == nSamples

        # Check if higher number is possible
        nSamples = 10
        vec2 = coll.random_select_compounds(nSamples)
        assert len(vec2) != nSamples
        assert len(vec2) == coll.count("{}")

    def test_count_by_dict(self):
        # Make sure the DB is clean in order to allow for accurate counts.
        self.manager.wipe()
        self.manager.init()
        # Setup
        coll = self.manager.get_collection("compounds")
        id1 = db.ID()
        id2 = db.ID()
        _ = db.Compound.make([id1], coll)
        _ = db.Compound.make([id1], coll)
        _ = db.Compound.make([id2], coll)

        query = {"structures": {"$eq": {"$oid": id1.string()}}}
        result = coll.count(dumps(query))

        # Check
        assert result == 2

    def test_loop(self):
        # Make sure the DB is clean in order to allow for accurate counts.
        self.manager.wipe()
        self.manager.init()
        # Setup
        coll = self.manager.get_collection("compounds")
        id1 = db.ID()
        id2 = db.ID()
        _ = db.Compound.make([id1], coll)
        _ = db.Compound.make([id1], coll)
        _ = db.Compound.make([id2], coll)

        query = {"structures": {"$eq": {"$oid": id1.string()}}}

        count = 0
        for comp in coll.iterate_compounds(dumps(query)):
            count += 1
            comp.link(coll)
            assert comp.has_structure(id1)

        # Check
        assert count == 2
