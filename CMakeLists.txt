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

add_versioned_package("gh:intel/cpp-baremetal-concurrency#44943df")
add_versioned_package("gh:intel/cpp-std-extensions#4190978")
add_versioned_package("gh:intel/cpp-baremetal-senders-and-receivers#e910b44")
add_versioned_package("gh:intel/generic-register-operation-optimizer#541f14c")
add_versioned_package("gh:caisselabs/stm32-groov#6769da8")
#add_versioned_package("gh:intel/compile-time-init-build")

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
