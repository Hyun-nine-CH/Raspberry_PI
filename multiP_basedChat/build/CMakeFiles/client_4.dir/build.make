# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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
CMAKE_SOURCE_DIR = /home/rasp/folder/multiP_basedChat

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/rasp/folder/multiP_basedChat/build

# Include any dependencies generated for this target.
include CMakeFiles/client_4.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/client_4.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/client_4.dir/flags.make

CMakeFiles/client_4.dir/clients/client_4.c.o: CMakeFiles/client_4.dir/flags.make
CMakeFiles/client_4.dir/clients/client_4.c.o: ../clients/client_4.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/rasp/folder/multiP_basedChat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/client_4.dir/clients/client_4.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/client_4.dir/clients/client_4.c.o -c /home/rasp/folder/multiP_basedChat/clients/client_4.c

CMakeFiles/client_4.dir/clients/client_4.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/client_4.dir/clients/client_4.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/rasp/folder/multiP_basedChat/clients/client_4.c > CMakeFiles/client_4.dir/clients/client_4.c.i

CMakeFiles/client_4.dir/clients/client_4.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/client_4.dir/clients/client_4.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/rasp/folder/multiP_basedChat/clients/client_4.c -o CMakeFiles/client_4.dir/clients/client_4.c.s

# Object files for target client_4
client_4_OBJECTS = \
"CMakeFiles/client_4.dir/clients/client_4.c.o"

# External object files for target client_4
client_4_EXTERNAL_OBJECTS =

client_4: CMakeFiles/client_4.dir/clients/client_4.c.o
client_4: CMakeFiles/client_4.dir/build.make
client_4: CMakeFiles/client_4.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/rasp/folder/multiP_basedChat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable client_4"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/client_4.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/client_4.dir/build: client_4

.PHONY : CMakeFiles/client_4.dir/build

CMakeFiles/client_4.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/client_4.dir/cmake_clean.cmake
.PHONY : CMakeFiles/client_4.dir/clean

CMakeFiles/client_4.dir/depend:
	cd /home/rasp/folder/multiP_basedChat/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/rasp/folder/multiP_basedChat /home/rasp/folder/multiP_basedChat /home/rasp/folder/multiP_basedChat/build /home/rasp/folder/multiP_basedChat/build /home/rasp/folder/multiP_basedChat/build/CMakeFiles/client_4.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/client_4.dir/depend

