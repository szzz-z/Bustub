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
include third_party/murmur3/CMakeFiles/bustub_murmur3.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include third_party/murmur3/CMakeFiles/bustub_murmur3.dir/compiler_depend.make

# Include the progress variables for this target.
include third_party/murmur3/CMakeFiles/bustub_murmur3.dir/progress.make

# Include the compile flags for this target's objects.
include third_party/murmur3/CMakeFiles/bustub_murmur3.dir/flags.make

third_party/murmur3/CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.o: third_party/murmur3/CMakeFiles/bustub_murmur3.dir/flags.make
third_party/murmur3/CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.o: ../third_party/murmur3/MurmurHash3.cpp
third_party/murmur3/CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.o: third_party/murmur3/CMakeFiles/bustub_murmur3.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/szzz/bustub/build_rel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object third_party/murmur3/CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.o"
	cd /home/szzz/bustub/build_rel/third_party/murmur3 && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT third_party/murmur3/CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.o -MF CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.o.d -o CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.o -c /home/szzz/bustub/third_party/murmur3/MurmurHash3.cpp

third_party/murmur3/CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.i"
	cd /home/szzz/bustub/build_rel/third_party/murmur3 && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/szzz/bustub/third_party/murmur3/MurmurHash3.cpp > CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.i

third_party/murmur3/CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.s"
	cd /home/szzz/bustub/build_rel/third_party/murmur3 && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/szzz/bustub/third_party/murmur3/MurmurHash3.cpp -o CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.s

# Object files for target bustub_murmur3
bustub_murmur3_OBJECTS = \
"CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.o"

# External object files for target bustub_murmur3
bustub_murmur3_EXTERNAL_OBJECTS =

lib/libbustub_murmur3.a: third_party/murmur3/CMakeFiles/bustub_murmur3.dir/MurmurHash3.cpp.o
lib/libbustub_murmur3.a: third_party/murmur3/CMakeFiles/bustub_murmur3.dir/build.make
lib/libbustub_murmur3.a: third_party/murmur3/CMakeFiles/bustub_murmur3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/szzz/bustub/build_rel/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library ../../lib/libbustub_murmur3.a"
	cd /home/szzz/bustub/build_rel/third_party/murmur3 && $(CMAKE_COMMAND) -P CMakeFiles/bustub_murmur3.dir/cmake_clean_target.cmake
	cd /home/szzz/bustub/build_rel/third_party/murmur3 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bustub_murmur3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
third_party/murmur3/CMakeFiles/bustub_murmur3.dir/build: lib/libbustub_murmur3.a
.PHONY : third_party/murmur3/CMakeFiles/bustub_murmur3.dir/build

third_party/murmur3/CMakeFiles/bustub_murmur3.dir/clean:
	cd /home/szzz/bustub/build_rel/third_party/murmur3 && $(CMAKE_COMMAND) -P CMakeFiles/bustub_murmur3.dir/cmake_clean.cmake
.PHONY : third_party/murmur3/CMakeFiles/bustub_murmur3.dir/clean

third_party/murmur3/CMakeFiles/bustub_murmur3.dir/depend:
	cd /home/szzz/bustub/build_rel && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/szzz/bustub /home/szzz/bustub/third_party/murmur3 /home/szzz/bustub/build_rel /home/szzz/bustub/build_rel/third_party/murmur3 /home/szzz/bustub/build_rel/third_party/murmur3/CMakeFiles/bustub_murmur3.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : third_party/murmur3/CMakeFiles/bustub_murmur3.dir/depend

