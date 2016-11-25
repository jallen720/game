# Project info
cmake_minimum_required(VERSION "3.2.2" FATAL_ERROR)
project("Game")
set(CMAKE_PATH "$ENV{CMAKE_PATH}")

# Utilities
include("${CMAKE_PATH}/macros/configure_project.cmake")
include("${CMAKE_PATH}/macros/module_dependency.cmake")
include("${CMAKE_PATH}/macros/add_bin.cmake")
include("${CMAKE_PATH}/macros/test_module_dependency.cmake")
include("${CMAKE_PATH}/macros/add_bin_tests.cmake")

# Configuration
configure_project()

# App binary
module_dependency("Cpp_Utils")
module_dependency("GLFW3")
module_dependency("Nito")
add_bin()

# Tests
test_module_dependency("GoogleTest")
add_bin_tests()
