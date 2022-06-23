__copyright__ = """This file is part of SCINE Utilities.
This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

from conans import ConanFile


class TestPackageConan(ConanFile):
    def build(self):
        pass

    def test(self):
        if self.options["scine_database"].python:
            self.output.info("Trying to import 'scine_database'")
            import scine_database
            self.output.info("Import worked")
