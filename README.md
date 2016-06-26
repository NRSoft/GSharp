# GSharp
Library for interpreting G# macro-programming CNC language into plain G-code lines

Version 1.0

Description
-----------
The library provides a line-by-line interpreter of the G# program.
G# is a macro-programming language for CNC control which extends traditional G-code programming
by allowing to use variables, conditions, loop executions and subroutine calls.
The G# program can contain both plain G-code lines as well as the extended commands.

G# syntax is a subset of [LinuxCNC](https://github.com/LinuxCNC/linuxcnc) and
[RS274/NGC](https://www.nist.gov/customcf/get_pdf.cfm?pub_id=823374) and was designed to provide high
level of compatibility to these de-facto standards of the open-source CNC macro-programming.
The differences are stated below, in all other cases the syntax should match.

The library does not control any machine directly, but generates plain G-code command lines one-by one
after issuing the corresponding calls. As such the output may be used by any CNC controller which understands
plain G-codes (GRBL, Shapeoko, TinyG, etc.).
These G-code lines can be either fed directly into a CNC controller or used for reviewing or visulisation
of the job by control software.

**Major differences to the LinuxCNC control language syntax:**

*Limitations*
 * does NOT support named_parameters and correspondingly no EXIST[arg] funcion.
 * as a result O-endsub and O-return commands can NOT store return value into the '_value' parameter.
 These commands can still return a value, but it will be stored in the parameter #5000.
 * O-call can NOT be issued to a subroutine located in a separate file.
 * in the current version all numbered parameters are volatile (persistence is planned for future releases).
 * (PROBE) comments are ignored as not relevant for interpreting (should be managed by the machine control system).

*Improvements*
 * % demarcation lines can appear anywhere in the program. The first % marks the start, the second - stop of the execution.
 * O-subs can be located anywhere in the code: they are executed only if called and jumped over in all other cases.
 * comments can be anywhere in any line (except for % lines, where they can appear only after % character),
 they get processed accordingly and removed before parsing.
 * comments can contain pairs of brackets "()" inside, but not a non-matched single bracket.
 * several active comments can be present in the same line, if they are of different types, they all will be reported.

Requirements
------------
Current version uses CMake v3.1+ to build the library, generate the usage example and run the unit tests.
The repository also contains [Code::Blocks](http://www.codeblocks.org/) project files.
To build the library you will need a C++11 compatible compiler (e.g. [G++](https://gcc.gnu.org/) v4.8 or higher).
There are no external dependencies if the goal is to produce the release version of the library.

To compile the library using CMake navigate to the top directory and run:

    cmake .
    make

The generated library will be located under the 'lib' sub-directory. The header files are under the 'include' sub-directory.

Usage
-----
The top directory contains the 'example.cpp' file, which demonstrates how to use the library.
It opens a G# program from a file, interpretes it line-by-line and stores the output
(plain G-code lines) into another file. As such it can be used as a stand-alone converter.
The `make` command will generate 'gs2g' binary from the 'example.cpp', which can be executed in command line as following:

    ./gs2g <input_file> <output_file>

Test
----
Unit tests are also provided, they use [googletest](https://github.com/google/googletest)
as the testing platform. For linking consistency the source code of googletest v1.7 is also provided.
Running the `make` command will automatically generate required gtest libraries.
Alternatively you can use corresponding Code::Blocks project file in 'external/gtest' directory to build the libraries 
(select target "All" and recompile).

To run unit tests:

    make test

Alternatively you can run them directly as gtest bianry:

    ./gsharp_test

The library has been tested on Linux and Windows machines only.
Please let us know if there are any issues compiling and running it on other OS platforms.

Windows
-------
Follow the instruction in the win/README.md to build the library and GSharp converter in Windows environment.

License
-------
The program is distributed under BSD license. See LICENSE file for details.

Issues
--------
Please report any bugs or incompatibilities with LinuxCNC syntax directly to the issue tracker of this project.
