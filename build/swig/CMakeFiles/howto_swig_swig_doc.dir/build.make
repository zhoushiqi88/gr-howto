# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zhou/src/gr-howto

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zhou/src/gr-howto/build

# Utility rule file for howto_swig_swig_doc.

# Include the progress variables for this target.
include swig/CMakeFiles/howto_swig_swig_doc.dir/progress.make

swig/CMakeFiles/howto_swig_swig_doc: swig/howto_swig_doc.i


swig/howto_swig_doc.i: swig/howto_swig_doc_swig_docs/xml/index.xml
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/zhou/src/gr-howto/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating python docstrings for howto_swig_doc"
	cd /home/zhou/src/gr-howto/docs/doxygen && /usr/bin/python2 -B /home/zhou/src/gr-howto/docs/doxygen/swig_doc.py /home/zhou/src/gr-howto/build/swig/howto_swig_doc_swig_docs/xml /home/zhou/src/gr-howto/build/swig/howto_swig_doc.i

swig/howto_swig_doc_swig_docs/xml/index.xml: swig/_howto_swig_doc_tag
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/zhou/src/gr-howto/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating doxygen xml for howto_swig_doc docs"
	cd /home/zhou/src/gr-howto/build/swig && ./_howto_swig_doc_tag
	cd /home/zhou/src/gr-howto/build/swig && /usr/bin/doxygen /home/zhou/src/gr-howto/build/swig/howto_swig_doc_swig_docs/Doxyfile

howto_swig_swig_doc: swig/CMakeFiles/howto_swig_swig_doc
howto_swig_swig_doc: swig/howto_swig_doc.i
howto_swig_swig_doc: swig/howto_swig_doc_swig_docs/xml/index.xml
howto_swig_swig_doc: swig/CMakeFiles/howto_swig_swig_doc.dir/build.make

.PHONY : howto_swig_swig_doc

# Rule to build all files generated by this target.
swig/CMakeFiles/howto_swig_swig_doc.dir/build: howto_swig_swig_doc

.PHONY : swig/CMakeFiles/howto_swig_swig_doc.dir/build

swig/CMakeFiles/howto_swig_swig_doc.dir/clean:
	cd /home/zhou/src/gr-howto/build/swig && $(CMAKE_COMMAND) -P CMakeFiles/howto_swig_swig_doc.dir/cmake_clean.cmake
.PHONY : swig/CMakeFiles/howto_swig_swig_doc.dir/clean

swig/CMakeFiles/howto_swig_swig_doc.dir/depend:
	cd /home/zhou/src/gr-howto/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zhou/src/gr-howto /home/zhou/src/gr-howto/swig /home/zhou/src/gr-howto/build /home/zhou/src/gr-howto/build/swig /home/zhou/src/gr-howto/build/swig/CMakeFiles/howto_swig_swig_doc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : swig/CMakeFiles/howto_swig_swig_doc.dir/depend

