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
include tools/btree_bench/CMakeFiles/btree-bench.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tools/btree_bench/CMakeFiles/btree-bench.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/btree_bench/CMakeFiles/btree-bench.dir/progress.make

# Include the compile flags for this target's objects.
include tools/btree_bench/CMakeFiles/btree-bench.dir/flags.make

tools/btree_bench/CMakeFiles/btree-bench.dir/btree_bench.cpp.o: tools/btree_bench/CMakeFiles/btree-bench.dir/flags.make
tools/btree_bench/CMakeFiles/btree-bench.dir/btree_bench.cpp.o: ../tools/btree_bench/btree_bench.cpp
tools/btree_bench/CMakeFiles/btree-bench.dir/btree_bench.cpp.o: tools/btree_bench/CMakeFiles/btree-bench.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/szzz/bustub-2023spring/build_rel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tools/btree_bench/CMakeFiles/btree-bench.dir/btree_bench.cpp.o"
	cd /home/szzz/bustub-2023spring/build_rel/tools/btree_bench && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/btree_bench/CMakeFiles/btree-bench.dir/btree_bench.cpp.o -MF CMakeFiles/btree-bench.dir/btree_bench.cpp.o.d -o CMakeFiles/btree-bench.dir/btree_bench.cpp.o -c /home/szzz/bustub-2023spring/tools/btree_bench/btree_bench.cpp

tools/btree_bench/CMakeFiles/btree-bench.dir/btree_bench.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/btree-bench.dir/btree_bench.cpp.i"
	cd /home/szzz/bustub-2023spring/build_rel/tools/btree_bench && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/szzz/bustub-2023spring/tools/btree_bench/btree_bench.cpp > CMakeFiles/btree-bench.dir/btree_bench.cpp.i

tools/btree_bench/CMakeFiles/btree-bench.dir/btree_bench.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/btree-bench.dir/btree_bench.cpp.s"
	cd /home/szzz/bustub-2023spring/build_rel/tools/btree_bench && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/szzz/bustub-2023spring/tools/btree_bench/btree_bench.cpp -o CMakeFiles/btree-bench.dir/btree_bench.cpp.s

# Object files for target btree-bench
btree__bench_OBJECTS = \
"CMakeFiles/btree-bench.dir/btree_bench.cpp.o"

# External object files for target btree-bench
btree__bench_EXTERNAL_OBJECTS =

bin/bustub-btree-bench: tools/btree_bench/CMakeFiles/btree-bench.dir/btree_bench.cpp.o
bin/bustub-btree-bench: tools/btree_bench/CMakeFiles/btree-bench.dir/build.make
bin/bustub-btree-bench: lib/libbustub.a
bin/bustub-btree-bench: lib/libbustub_murmur3.a
bin/bustub-btree-bench: lib/libduckdb_pg_query.a
bin/bustub-btree-bench: lib/libfmt.a
bin/bustub-btree-bench: lib/libfort.a
bin/bustub-btree-bench: tools/btree_bench/CMakeFiles/btree-bench.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/szzz/bustub-2023spring/build_rel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/bustub-btree-bench"
	cd /home/szzz/bustub-2023spring/build_rel/tools/btree_bench && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/btree-bench.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/btree_bench/CMakeFiles/btree-bench.dir/build: bin/bustub-btree-bench
.PHONY : tools/btree_bench/CMakeFiles/btree-bench.dir/build

tools/btree_bench/CMakeFiles/btree-bench.dir/clean:
	cd /home/szzz/bustub-2023spring/build_rel/tools/btree_bench && $(CMAKE_COMMAND) -P CMakeFiles/btree-bench.dir/cmake_clean.cmake
.PHONY : tools/btree_bench/CMakeFiles/btree-bench.dir/clean

tools/btree_bench/CMakeFiles/btree-bench.dir/depend:
	cd /home/szzz/bustub-2023spring/build_rel && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/szzz/bustub-2023spring /home/szzz/bustub-2023spring/tools/btree_bench /home/szzz/bustub-2023spring/build_rel /home/szzz/bustub-2023spring/build_rel/tools/btree_bench /home/szzz/bustub-2023spring/build_rel/tools/btree_bench/CMakeFiles/btree-bench.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/btree_bench/CMakeFiles/btree-bench.dir/depend
