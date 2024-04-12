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
CMAKE_SOURCE_DIR = /home/szzz/bustub

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/szzz/bustub/build_rel

# Include any dependencies generated for this target.
include tools/shell/CMakeFiles/shell.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tools/shell/CMakeFiles/shell.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/shell/CMakeFiles/shell.dir/progress.make

# Include the compile flags for this target's objects.
include tools/shell/CMakeFiles/shell.dir/flags.make

tools/shell/CMakeFiles/shell.dir/shell.cpp.o: tools/shell/CMakeFiles/shell.dir/flags.make
tools/shell/CMakeFiles/shell.dir/shell.cpp.o: ../tools/shell/shell.cpp
tools/shell/CMakeFiles/shell.dir/shell.cpp.o: tools/shell/CMakeFiles/shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/szzz/bustub/build_rel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tools/shell/CMakeFiles/shell.dir/shell.cpp.o"
	cd /home/szzz/bustub/build_rel/tools/shell && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/shell/CMakeFiles/shell.dir/shell.cpp.o -MF CMakeFiles/shell.dir/shell.cpp.o.d -o CMakeFiles/shell.dir/shell.cpp.o -c /home/szzz/bustub/tools/shell/shell.cpp

tools/shell/CMakeFiles/shell.dir/shell.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/shell.dir/shell.cpp.i"
	cd /home/szzz/bustub/build_rel/tools/shell && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/szzz/bustub/tools/shell/shell.cpp > CMakeFiles/shell.dir/shell.cpp.i

tools/shell/CMakeFiles/shell.dir/shell.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/shell.dir/shell.cpp.s"
	cd /home/szzz/bustub/build_rel/tools/shell && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/szzz/bustub/tools/shell/shell.cpp -o CMakeFiles/shell.dir/shell.cpp.s

# Object files for target shell
shell_OBJECTS = \
"CMakeFiles/shell.dir/shell.cpp.o"

# External object files for target shell
shell_EXTERNAL_OBJECTS =

bin/bustub-shell: tools/shell/CMakeFiles/shell.dir/shell.cpp.o
bin/bustub-shell: tools/shell/CMakeFiles/shell.dir/build.make
bin/bustub-shell: lib/libbustub.a
bin/bustub-shell: lib/libbustub_linenoise.a
bin/bustub-shell: lib/libutf8proc.a
bin/bustub-shell: lib/libbustub_murmur3.a
bin/bustub-shell: lib/libduckdb_pg_query.a
bin/bustub-shell: lib/libfmt.a
bin/bustub-shell: lib/libfort.a
bin/bustub-shell: tools/shell/CMakeFiles/shell.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/szzz/bustub/build_rel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/bustub-shell"
	cd /home/szzz/bustub/build_rel/tools/shell && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/shell.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/shell/CMakeFiles/shell.dir/build: bin/bustub-shell
.PHONY : tools/shell/CMakeFiles/shell.dir/build

tools/shell/CMakeFiles/shell.dir/clean:
	cd /home/szzz/bustub/build_rel/tools/shell && $(CMAKE_COMMAND) -P CMakeFiles/shell.dir/cmake_clean.cmake
.PHONY : tools/shell/CMakeFiles/shell.dir/clean

tools/shell/CMakeFiles/shell.dir/depend:
	cd /home/szzz/bustub/build_rel && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/szzz/bustub /home/szzz/bustub/tools/shell /home/szzz/bustub/build_rel /home/szzz/bustub/build_rel/tools/shell /home/szzz/bustub/build_rel/tools/shell/CMakeFiles/shell.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/shell/CMakeFiles/shell.dir/depend

