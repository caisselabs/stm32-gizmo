#
# Copyright (c) 2025 Michael Caisse
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.25)

project(
  stm32-gizmo
  VERSION 0.1.0
  LANGUAGES CXX C ASM
  )

include(cmake/get_cpm.cmake)
cpmaddpackage("gh:intel/cicd-repo-infrastructure#dev")

add_versioned_package(
    NAME cpp-baremetal-concurrency
    SOURCE_DIR /home/mjcaisse/sandbox/cpp-baremetal-concurrency
    GIT_TAG HEAD)
#add_versioned_package("gh:intel/cpp-baremetal-concurrency#09c043b")

add_versioned_package(
    NAME cpp-std-extensions
    SOURCE_DIR /home/mjcaisse/sandbox/cpp-std-extensions
    GIT_TAG HEAD)
#add_versioned_package("gh:intel/cpp-std-extensions#ff96a19")

add_versioned_package(
    NAME cpp-baremetal-senders-and-receivers
    SOURCE_DIR /home/mjcaisse/sandbox/cpp-baremetal-senders-and-receivers
    GIT_TAG HEAD)
#add_versioned_package("gh:intel/cpp-baremetal-senders-and-receivers#fee82aa")

add_versioned_package(
    NAME generic-register-operation-optimizer
    SOURCE_DIR /home/mjcaisse/sandbox/generic-register-operation-optimizer
    GIT_TAG HEAD)
#add_versioned_package("gh:intel/generic-register-operation-optimizer#c056dee")

add_versioned_package(
    NAME stm32-groov
    SOURCE_DIR /home/mjcaisse/sandbox/stm32-groov
    GIT_TAG HEAD)
#add_versioned_package("gh:caisselabs/stm32-groov#f1f39f7")
#add_versioned_package("gh:intel/compile-time-init-build")



# --------------------------------------------------------------------------
# --------------------------------------------------------------------------
add_executable(test_test
  src/main.cpp
)
target_include_directories(test_test
  PRIVATE
  src/
)
# the detection used by baremetal concurrency results in determining
# that this target is hosted. Just force with define.
target_link_libraries_system(test_test PUBLIC async groov stm32-groov)
# --------------------------------------------------------------------------
# --------------------------------------------------------------------------


# --------------------------------------------------
# --------------------------------------------------
add_executable(button_demo
  src/startup/startup_gcc.s
  src/button/button_setup.cpp
  src/button/button.cpp
)
target_include_directories(button_demo
  PRIVATE
  src/
)
# the detection used by baremetal concurrency results in determining
# that this target is hosted. Just force with define.
target_compile_definitions(button_demo PUBLIC SIMULATE_FREESTANDING)
target_link_libraries_system(button_demo PUBLIC async groov stm32-groov)
# --------------------------------------------------
# --------------------------------------------------
