# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

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
CMAKE_SOURCE_DIR = /opt/chromium/src/third_party/abseil-cpp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /opt/chromium/src/third_party/abseil-cpp

# Include any dependencies generated for this target.
include absl/strings/CMakeFiles/cordz_handle.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include absl/strings/CMakeFiles/cordz_handle.dir/compiler_depend.make

# Include the progress variables for this target.
include absl/strings/CMakeFiles/cordz_handle.dir/progress.make

# Include the compile flags for this target's objects.
include absl/strings/CMakeFiles/cordz_handle.dir/flags.make

absl/strings/CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.o: absl/strings/CMakeFiles/cordz_handle.dir/flags.make
absl/strings/CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.o: absl/strings/internal/cordz_handle.cc
absl/strings/CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.o: absl/strings/CMakeFiles/cordz_handle.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/opt/chromium/src/third_party/abseil-cpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object absl/strings/CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.o"
	cd /opt/chromium/src/third_party/abseil-cpp/absl/strings && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT absl/strings/CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.o -MF CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.o.d -o CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.o -c /opt/chromium/src/third_party/abseil-cpp/absl/strings/internal/cordz_handle.cc

absl/strings/CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.i"
	cd /opt/chromium/src/third_party/abseil-cpp/absl/strings && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /opt/chromium/src/third_party/abseil-cpp/absl/strings/internal/cordz_handle.cc > CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.i

absl/strings/CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.s"
	cd /opt/chromium/src/third_party/abseil-cpp/absl/strings && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /opt/chromium/src/third_party/abseil-cpp/absl/strings/internal/cordz_handle.cc -o CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.s

# Object files for target cordz_handle
cordz_handle_OBJECTS = \
"CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.o"

# External object files for target cordz_handle
cordz_handle_EXTERNAL_OBJECTS =

absl/strings/libabsl_cordz_handle.a: absl/strings/CMakeFiles/cordz_handle.dir/internal/cordz_handle.cc.o
absl/strings/libabsl_cordz_handle.a: absl/strings/CMakeFiles/cordz_handle.dir/build.make
absl/strings/libabsl_cordz_handle.a: absl/strings/CMakeFiles/cordz_handle.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/opt/chromium/src/third_party/abseil-cpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libabsl_cordz_handle.a"
	cd /opt/chromium/src/third_party/abseil-cpp/absl/strings && $(CMAKE_COMMAND) -P CMakeFiles/cordz_handle.dir/cmake_clean_target.cmake
	cd /opt/chromium/src/third_party/abseil-cpp/absl/strings && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cordz_handle.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
absl/strings/CMakeFiles/cordz_handle.dir/build: absl/strings/libabsl_cordz_handle.a
.PHONY : absl/strings/CMakeFiles/cordz_handle.dir/build

absl/strings/CMakeFiles/cordz_handle.dir/clean:
	cd /opt/chromium/src/third_party/abseil-cpp/absl/strings && $(CMAKE_COMMAND) -P CMakeFiles/cordz_handle.dir/cmake_clean.cmake
.PHONY : absl/strings/CMakeFiles/cordz_handle.dir/clean

absl/strings/CMakeFiles/cordz_handle.dir/depend:
	cd /opt/chromium/src/third_party/abseil-cpp && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /opt/chromium/src/third_party/abseil-cpp /opt/chromium/src/third_party/abseil-cpp/absl/strings /opt/chromium/src/third_party/abseil-cpp /opt/chromium/src/third_party/abseil-cpp/absl/strings /opt/chromium/src/third_party/abseil-cpp/absl/strings/CMakeFiles/cordz_handle.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : absl/strings/CMakeFiles/cordz_handle.dir/depend

