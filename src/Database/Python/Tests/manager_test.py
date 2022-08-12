# -*- coding: utf-8 -*-
__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

import scine_utilities as utils
import scine_database as db
import unittest
import os


class ManagerTest(unittest.TestCase):
    def setUp(self):
        self.manager = db.Manager()
        self.manager.credentials.hostname = os.environ.get(
            'TEST_MONGO_DB_IP', '127.0.0.1')
        self.manager.credentials.port = int(os.environ.get('TEST_MONGO_DB_PORT', 27017))
        self.manager.credentials.database_name = "unittest_db_ManagerTest"
        self.manager.connect()
        self.manager.init()

    def tearDown(self):
        self.manager.wipe()

    def test_wrong_credentials(self):
        manager2 = db.Manager()
        manager2.credentials.hostname = 'THERE_AINT_NO_HOST_HERE'
        manager2.credentials.database_name = "unittest_db_AaBbCc"
        self.assertRaises(RuntimeError, lambda: manager2.connect())

    def test_credentials_getter_working(self):
        manager2 = db.Manager()
        test_port = os.environ.get('TEST_MONGO_DB_PORT', 27017)
        credentials = db.Credentials(
            "THERE_AINT_NO_HOST_HERE", test_port, "unittest_db_AaBbCc")
        manager2.set_credentials(credentials)
        copy = manager2.get_credentials()
        assert credentials.hostname == copy.hostname
        assert credentials.port == copy.port
        assert credentials.database_name == copy.database_name
        assert credentials.username == copy.username
        assert credentials.password == copy.password
        assert credentials.auth_database == copy.auth_database

    def test_credentials_comparison(self):
        test_port = os.environ.get('TEST_MONGO_DB_PORT', 27017)
        credentials = db.Credentials(
            "THERE_AINT_NO_HOST_HERE", test_port, "unittest_db_AaBbCc")
        credentials1 = db.Credentials(
            "THERE_AINT_NO_HOST_HERE", test_port, "unittest_db_AaBbCc")
        credentials2 = db.Credentials(
            "THERE_AINT_NO_OTHER_HOST_HERE", test_port, "unittest_db_AaBbCc")
        assert credentials == credentials1
        assert credentials != credentials2
        assert credentials1 != credentials2

    def test_db_name(self):
        manager = db.Manager()
        test_ip = os.environ.get('TEST_MONGO_DB_IP', '127.0.0.1')
        test_port = os.environ.get('TEST_MONGO_DB_PORT', 27017)
        test_db = db.Credentials(test_ip, int(test_port), "unittest_db_AaBbCc")
        manager.set_credentials(test_db)
        manager.connect()
        manager.init()
        assert manager.is_connected()
        assert manager.get_database_name() == "unittest_db_AaBbCc"
        # Check collections
        assert manager.has_collection("structures")
        assert manager.has_collection("calculations")
        assert manager.has_collection("properties")
        assert manager.has_collection("compounds")
        assert manager.has_collection("reactions")
        assert manager.has_collection("elementary_steps")
        manager.wipe()
        # Swap DB name
        manager.set_database_name("unittest_db_DdEeFf")
        manager.init()
        assert manager.get_database_name() != "unittest_db_AaBbCc"
        assert manager.get_database_name() == "unittest_db_DdEeFf"
        # Check collections
        assert manager.has_collection("structures")
        assert manager.has_collection("calculations")
        assert manager.has_collection("properties")
        assert manager.has_collection("compounds")
        assert manager.has_collection("reactions")
        assert manager.has_collection("elementary_steps")
        manager.wipe()

    def test_connection_credentials_init_and_wipe(self):
        manager = db.Manager()
        test_ip = os.environ.get('TEST_MONGO_DB_IP', '127.0.0.1')
        test_port = os.environ.get('TEST_MONGO_DB_PORT', 27017)
        test_db = db.Credentials(test_ip, int(test_port), "unittest_db_AaBbCc")
        manager.set_credentials(test_db)
        manager.connect()
        manager.init()
        assert manager.is_connected()
        # Check collections
        assert manager.has_collection("structures")
        assert manager.has_collection("calculations")
        assert manager.has_collection("properties")
        assert manager.has_collection("compounds")
        assert manager.has_collection("reactions")
        assert manager.has_collection("elementary_steps")
        # Wipe and check again
        manager.wipe()
        assert not manager.has_collection("structures")
        assert not manager.has_collection("calculations")
        assert not manager.has_collection("properties")
        assert not manager.has_collection("compounds")
        assert not manager.has_collection("reactions")
        assert not manager.has_collection("elementary_steps")
        # Init and check again
        manager.init()
        assert manager.has_collection("structures")
        assert manager.has_collection("calculations")
        assert manager.has_collection("properties")
        assert manager.has_collection("compounds")
        assert manager.has_collection("reactions")
        assert manager.has_collection("elementary_steps")
        # Clean
        manager.wipe()
        manager.disconnect()
        assert not manager.is_connected()

    def test_remote_wipe(self):
        manager = db.Manager()
        test_ip = os.environ.get('TEST_MONGO_DB_IP', '127.0.0.1')
        test_port = os.environ.get('TEST_MONGO_DB_PORT', 27017)
        test_db = db.Credentials(test_ip, int(test_port), "unittest_db_AaBbCc")
        manager.set_credentials(test_db)
        manager.connect()
        manager.init()
        assert manager.is_connected()
        # Check collections
        assert manager.has_collection("structures")
        assert manager.has_collection("calculations")
        assert manager.has_collection("properties")
        assert manager.has_collection("compounds")
        assert manager.has_collection("reactions")
        assert manager.has_collection("elementary_steps")
        # Wipe and check again
        manager.disconnect()
        manager.wipe(True)
        manager.connect()
        assert not manager.has_collection("structures")
        assert not manager.has_collection("calculations")
        assert not manager.has_collection("properties")
        assert not manager.has_collection("compounds")
        assert not manager.has_collection("reactions")
        assert not manager.has_collection("elementary_steps")

    def test_server_time(self):
        manager = db.Manager()
        self.assertRaises(RuntimeError, lambda: manager.server_time())
        test_ip = os.environ.get('TEST_MONGO_DB_IP', '127.0.0.1')
        test_port = os.environ.get('TEST_MONGO_DB_PORT', 27017)
        test_db = db.Credentials(test_ip, int(test_port), "unittest_db_AaBbCc")
        manager.set_credentials(test_db)
        manager.connect()
        manager.server_time()

    def test_reconnect(self):
        manager = db.Manager()
        test_ip = os.environ.get('TEST_MONGO_DB_IP', '127.0.0.1')
        test_port = os.environ.get('TEST_MONGO_DB_PORT', 27017)
        test_db = db.Credentials(test_ip, int(test_port), "unittest_db_AaBbCc")
        manager.set_credentials(test_db)
        manager.connect()
        manager.init()
        manager.disconnect()
        manager.connect()

    def test_get_collection(self):
        self.manager.init()
        _ = self.manager.get_collection("structures")

    def test_check_version(self):
        self.manager.init()
        assert self.manager.version_matches_wrapper()

    def test_get_version(self):
        self.manager.init()
        v = self.manager.get_db_version()
        assert v[0] == db.database_version.major
        assert v[1] == db.database_version.minor
        assert v[2] == db.database_version.patch

    def test_get_collection_fails(self):
        manager2 = db.Manager()
        self.assertRaises(
            RuntimeError, lambda: manager2.get_collection("nopenopenope"))

    def test_expected_db_fails(self):
        manager2 = db.Manager()
        self.assertRaises(RuntimeError, lambda: manager2.connect(True))

    def test_has_collection_fails(self):
        manager2 = db.Manager()
        self.assertRaises(
            RuntimeError, lambda: manager2.has_collection("nopenopenope")())

    def test_wipe_fails(self):
        manager2 = db.Manager()
        self.assertRaises(RuntimeError, lambda: manager2.wipe())

    def test_init_fails(self):
        manager2 = db.Manager()
        self.assertRaises(RuntimeError, lambda: manager2.init())
