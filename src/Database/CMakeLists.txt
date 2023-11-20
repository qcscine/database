# Load source file lists
include(Files.cmake)


# Find/Import dependecies
include(ImportUtilsOS)
import_utils_os()
include(AddEigen)
include(ImportMongoCxx)
import_mongocxx()

add_library(Database ${DATABASE_MODULE_FILES})
if(NOT BUILD_SHARED_LIBS)
  set_target_properties(Database PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
target_include_directories(Database PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/Scine>
)

set_target_properties(Database PROPERTIES
  OUTPUT_NAME database
  CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_link_libraries(Database PUBLIC Scine::UtilsOS PRIVATE MongoDBCXX)

# Add namespaced aliases
add_library(Scine::Database ALIAS Database)

# -- Define Install
# Headers
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Database/
  DESTINATION include/Scine/Database
  FILES_MATCHING PATTERN "*.h"
)

# Targets
install(
  TARGETS Database
  EXPORT databaseTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# Component-level cmake package files
scine_install_component_cmake_files(
  COMPONENT  ScineDatabase
  EXPORT_NAME databaseTargets
)

# Tests
if(SCINE_BUILD_TESTS)
  include(ImportGTest)
  import_gtest()
  add_executable(Database_tests ${DATABASE_TEST_FILES})
  target_link_libraries(Database_tests
    PUBLIC
      GTest::Main
      GMock::GMock
      Scine::Database
    PRIVATE
      MongoDBCXX
  )
  add_test(
    NAME Database 
    COMMAND Database_tests --gtest_output=xml:gtest_report.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
  set(
    TEST_MONGO_DB_IP "localhost" CACHE
    STRING "The IP of the MongoDB the unittests will use."
  )
  target_compile_definitions(Database_tests PRIVATE
    TEST_MONGO_DB_IP="${TEST_MONGO_DB_IP}"
  )
  set(
    TEST_MONGO_DB_PORT "27017" CACHE
    STRING "The port of the MongoDB the unittests will use."
  )
  target_compile_definitions(Database_tests PRIVATE
    TEST_MONGO_DB_PORT="${TEST_MONGO_DB_PORT}"
  )
endif()

# Python Bindings
if(SCINE_BUILD_PYTHON_BINDINGS)
  include(ImportPybind11)
  import_pybind11()
  set(PYBIND11_PYTHON_VERSION ${PYTHONVERSION})
  pybind11_add_module(scine_database ${DATABASE_PYTHON_FILES})
  set_target_properties(scine_database PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/scine_database
  )
  target_link_libraries(scine_database PRIVATE Scine::Database)
  target_include_directories(scine_database PRIVATE
    $<TARGET_PROPERTY:MongoDBCXX,INTERFACE_INCLUDE_DIRECTORIES>
  )

  # Installation details
  install(CODE
    "execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install --prefix=${CMAKE_INSTALL_PREFIX} --upgrade --no-deps ${CMAKE_CURRENT_BINARY_DIR}
                     RESULT_VARIABLE retcode)
     if(NOT \$\{retcode\} EQUAL 0)
       message(FATAL_ERROR \"Fatal error when installing Python module using PIP.\")
     endif()"
  )

  # Copy the __init__ file and rename it appropriately
  file(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/Python/pkginit.py
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/scine_database
  )
  file(
    RENAME ${CMAKE_CURRENT_BINARY_DIR}/scine_database/pkginit.py
    ${CMAKE_CURRENT_BINARY_DIR}/scine_database/__init__.py
  )

  # Figure out which targets we have to copy into the python package
  include(TargetLibName)
  set(_py_targets_to_copy Scine::Core)
  target_lib_type(Scine::UtilsOS _utils_libtype)
  if(_utils_libtype STREQUAL "SHARED_LIBRARY")
    list(APPEND _py_targets_to_copy Scine::UtilsOS)
  endif()
  unset(_utils_libtype)
  if(BUILD_SHARED_LIBS)
    list(APPEND _py_targets_to_copy Scine::Database)
  endif()

  # Generate generator expressions for each target and figure out filenames
  # for the python setup file
  set(database_PY_DEPS "")
  foreach(target ${_py_targets_to_copy})
    list(APPEND _py_target_gen_exprs "\$<TARGET_FILE:${target}>")
    target_lib_filename(${target} _target_filename)
    string(APPEND database_PY_DEPS ", \"${_target_filename}\"")
  endforeach()

  add_custom_command(TARGET Database POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${_py_target_gen_exprs} ${CMAKE_CURRENT_BINARY_DIR}/scine_database
    COMMENT "Copying dependent shared libraries into python package directory"
  )

  # copy pure python files
  add_custom_command(TARGET Database POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${DATABASE_PURE_PYTHON_FILES} ${CMAKE_CURRENT_BINARY_DIR}/scine_database
    COMMENT "Copying pure Python files into Python package directory"
  )

  # Copy the setup file and README.rst into the binary dir so you can install
  # using pip or make a distribution with the compiled binary
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Python/setup.py
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py
    @ONLY
  )
  unset(database_PY_DEPS)
  file(
    COPY ${PROJECT_SOURCE_DIR}/README.rst
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
  )

  # set rpath
  if(APPLE)
    set_target_properties(scine_database PROPERTIES
      BUILD_WITH_INSTALL_RPATH ON
      INSTALL_RPATH "@loader_path;@loader_path/../lib"
    )
  elseif(UNIX)
    set_target_properties(scine_database PROPERTIES
      BUILD_WITH_INSTALL_RPATH ON
      INSTALL_RPATH "\$ORIGIN;\$ORIGIN/../lib"
    )
  endif()

  # Copy license files
  file(COPY ${PROJECT_SOURCE_DIR}/LICENSE.txt  ${EXTERNAL_LICENSE_FILES}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/scine_database
  )
  file(COPY ${EXTERNAL_LICENSE_FILES}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/external_licenses
  )

  # Typing stubs
  include(FindPythonModule)
  if(scine-utils-os_BINARY_DIR)
    set(UTILS_PYTHONPATH ${scine-utils-os_BINARY_DIR}/src/Utils)
  else()
    find_python_module(scine_utilities)
    if(PY_SCINE_UTILITIES)
      set(UTILS_PYTHONPATH ${PY_SCINE_UTILITIES})
    else()
      message("Not generating type stubs: Could not find scine_utilities")
    endif()
  endif()
  find_python_module(pybind11_stubgen)
  if(NOT PY_PYBIND11_STUBGEN)
    message(STATUS "Not generating type stubs: Could not find pybind11_stubgen")
  endif()
  if(PY_PYBIND11_STUBGEN AND UTILS_PYTHONPATH)
    add_custom_command(TARGET scine_database POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${UTILS_PYTHONPATH}:$ENV{PYTHONPATH} ${PYTHON_EXECUTABLE} -m pybind11_stubgen -o . --root-module-suffix \"\" --no-setup-py --bare-numpy-ndarray scine_database
      COMMENT "Generating python package typechecking stubs with pybind11-stubgen"
    )
  endif()

  # Tests
  if(SCINE_BUILD_TESTS)
    find_python_module(doctest)
    if(PY_DOCTEST AND UTILS_PYTHONPATH)
      add_test(
        NAME DatabasePythonDoctest
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Python/DocTest/doctesting.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
      set_tests_properties(DatabasePythonDoctest PROPERTIES
        ENVIRONMENT PYTHONPATH=${UTILS_PYTHONPATH}:${CMAKE_CURRENT_BINARY_DIR}:$ENV{PYTHONPATH}
      )
      add_test(
        NAME DatabasePythonTests
        COMMAND ${PYTHON_EXECUTABLE} -B -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/Python/Tests --junitxml=${CMAKE_CURRENT_BINARY_DIR}/pytest_report.xml
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
      set_tests_properties(DatabasePythonTests PROPERTIES
        ENVIRONMENT PYTHONPATH=${UTILS_PYTHONPATH}:${CMAKE_CURRENT_BINARY_DIR}:$ENV{PYTHONPATH}
      )
    endif()
  endif()
endif()
