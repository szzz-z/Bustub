# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/szzz/bustub-2023spring

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/szzz/bustub-2023spring/build_rel

# Include any dependencies generated for this target.
include test/CMakeFiles/trie_noncopy_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/trie_noncopy_test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/trie_noncopy_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/trie_noncopy_test.dir/flags.make

test/CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.o: test/CMakeFiles/trie_noncopy_test.dir/flags.make
test/CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.o: ../test/primer/trie_noncopy_test.cpp
test/CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.o: test/CMakeFiles/trie_noncopy_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/szzz/bustub-2023spring/build_rel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.o"
	cd /home/szzz/bustub-2023spring/build_rel/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.o -MF CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.o.d -o CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.o -c /home/szzz/bustub-2023spring/test/primer/trie_noncopy_test.cpp

test/CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.i"
	cd /home/szzz/bustub-2023spring/build_rel/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/szzz/bustub-2023spring/test/primer/trie_noncopy_test.cpp > CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.i

test/CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.s"
	cd /home/szzz/bustub-2023spring/build_rel/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/szzz/bustub-2023spring/test/primer/trie_noncopy_test.cpp -o CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.s

# Object files for target trie_noncopy_test
trie_noncopy_test_OBJECTS = \
"CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.o"

# External object files for target trie_noncopy_test
trie_noncopy_test_EXTERNAL_OBJECTS =

test/trie_noncopy_test: test/CMakeFiles/trie_noncopy_test.dir/primer/trie_noncopy_test.cpp.o
test/trie_noncopy_test: test/CMakeFiles/trie_noncopy_test.dir/build.make
test/trie_noncopy_test: lib/libbustub.a
test/trie_noncopy_test: lib/libgtest.a
test/trie_noncopy_test: lib/libgmock_main.a
test/trie_noncopy_test: lib/libbustub_murmur3.a
test/trie_noncopy_test: lib/libduckdb_pg_query.a
test/trie_noncopy_test: lib/libfmt.a
test/trie_noncopy_test: lib/libfort.a
test/trie_noncopy_test: lib/libgmock.a
test/trie_noncopy_test: lib/libgtest.a
test/trie_noncopy_test: test/CMakeFiles/trie_noncopy_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/szzz/bustub-2023spring/build_rel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable trie_noncopy_test"
	cd /home/szzz/bustub-2023spring/build_rel/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/trie_noncopy_test.dir/link.txt --verbose=$(VERBOSE)
	cd /home/szzz/bustub-2023spring/build_rel/test && /usr/bin/cmake -D TEST_TARGET=trie_noncopy_test -D TEST_EXECUTABLE=/home/szzz/bustub-2023spring/build_rel/test/trie_noncopy_test -D TEST_EXECUTOR= -D TEST_WORKING_DIR=/home/szzz/bustub-2023spring/build_rel/test -D "TEST_EXTRA_ARGS=--gtest_color=auto;--gtest_output=xml:/home/szzz/bustub-2023spring/build_rel/test/trie_noncopy_test.xml;--gtest_catch_exceptions=0" -D "TEST_PROPERTIES=TIMEOUT;120" -D TEST_PREFIX= -D TEST_SUFFIX= -D TEST_FILTER= -D NO_PRETTY_TYPES=FALSE -D NO_PRETTY_VALUES=FALSE -D TEST_LIST=trie_noncopy_test_TESTS -D CTEST_FILE=/home/szzz/bustub-2023spring/build_rel/test/trie_noncopy_test[1]_tests.cmake -D TEST_DISCOVERY_TIMEOUT=120 -D TEST_XML_OUTPUT_DIR= -P /usr/share/cmake-3.22/Modules/GoogleTestAddTests.cmake

# Rule to build all files generated by this target.
test/CMakeFiles/trie_noncopy_test.dir/build: test/trie_noncopy_test
.PHONY : test/CMakeFiles/trie_noncopy_test.dir/build

test/CMakeFiles/trie_noncopy_test.dir/clean:
	cd /home/szzz/bustub-2023spring/build_rel/test && $(CMAKE_COMMAND) -P CMakeFiles/trie_noncopy_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/trie_noncopy_test.dir/clean

test/CMakeFiles/trie_noncopy_test.dir/depend:
	cd /home/szzz/bustub-2023spring/build_rel && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/szzz/bustub-2023spring /home/szzz/bustub-2023spring/test /home/szzz/bustub-2023spring/build_rel /home/szzz/bustub-2023spring/build_rel/test /home/szzz/bustub-2023spring/build_rel/test/CMakeFiles/trie_noncopy_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/trie_noncopy_test.dir/depend

