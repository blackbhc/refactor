This documentation is for the code development of the project. I write this documentation for the
developers who are interested in the project. And I wish the documentation can cover all necessary
information for modification of the codes and further development.

Please feel free to raise an issue or give your valuable improvement suggestions for the project
and the documentations.

### Content <a name="content"></a>

1. <a href="#deps">Dependencies</a>
2. <a href="#files">Project Frame</a>
3. <a href="#unit_test">Unit Test</a>
4. <a href="#add_new_module">Add New Function Module</a>
5. <a href="#codes_structure">Codes Structure</a>

### Dependencies <a name="deps"></a><a href="#contents"><font size=4>(content)</font></a>

This project depends on the following tools or libraries:

- [gnu make](https://www.gnu.org/software/make/): the main build tool of the project.
- a c++ compiler which supports C++11: recommended ones are g++>=4.8.5 or clang++>=3.4, or icpx
  (the intel llvm-based compiler).
- [gsl](https://www.gnu.org/software/gsl/): the GNU Scientific Library.
- [hdf5](https://www.hdfgroup.org/solutions/hdf5/): the HDF5 library, which is used to store the analysis results.
- a `MPI` library: `mpich`, `openmpi` or `intel mpi` etc.

### Project Frame <a name="files"></a><a href="#contents"><font size=4>(content)</font></a>

- `Makefile`: the main makefile of the project, with some sub-makefiles in the `make-config` directory.
  - Note: due to the sub-makefiles, the `make` command should always be run in the root directory of the project.
- `src`: the source code directory.

  - `C_wrapper`: the directory include the public C wrapper APIs of the project.
  - `unit_test`: the directory of unit test wrapper codes, more details in the <a href="#unit_test">Unit Test</a> section.
  - `analysis`: the directory of the core analysis codes.
  - `tools`: define some useful tool functions and classes.
  - `output`: functions for analysis results output.
  - `parameter`: parameter file reading and parsing.
  - `engine`: the main virtual analysis engine, which is a wrapper of other modules.

- `documentation`: the directory of the developers' documentation and the details of the analysis codes.
- `test_dir`(optional): the temporary directory for the building and running unit test, which will
  be created by the `make test/mpi_test` command.
- `build`(optional): the temporary directory for the building of the project, which will be created by the
  `make` command.

### Unit Test <a name="unit_test"></a><a href="#contents"><font size=4>(content)</font></a>

#### 1. Run test in one command

Run `make test` or `make mpi_test` in the root directory of the project, you can test all unit tests.

The `test` target is for the unit test of the serial case, and the `mpi_test` for parallel case. It will
compile the code and run the unit test.

#### 2. Specify test units

By modifying the default value of the `TEST_TARGET` in the Makefile, or run `make test/mpi_test units=<unit to be tested>`
you can specify the which unit to be tested.

#### 3. Design unit test

`galotfa` is an OOP project, of which most functions are implemented as member functions of classes.
The unit test is built into each class (some standalone public function for test), which is conditionally
compiled that controlled by some macros ( which will be done by the command `make test/mpi_test`). To run any
unit test, what you need to do is define such macros before compiling, and call the test functions of an
object/namespace. .

So there are 5 steps to implement a new unit test for a new case:

1. define the corresponding macros, which will be done automatically by `make test/mpi_test` if you
   add such macro into the `TEST_TARGET` in the Makefile, for example: if you set `TEST_TARGET = <somthing>`,
   then the `debug_<somthing>` macro will be add to the `CXXFLAGS` by `-Ddebug_<somthing>`.

2. design the unit test for a new class or a new case in a existing class.

3. add a new `c++` file in the `src/test` directory, and define a function in which you call the test functions
   defined in the previous step.

4. In the `main` function of the `src/unit_test/test.cpp` file, include the new `c++` file create in the last step
   and call its function defined in the last step, with possible macro for conditional compiling for the unit test.

5. run `make test` or `make mpi_test` to compile and run the test.

Note: the `test` member function is just a wrapper of the designed unit tests, and you can split them
into more functions with possible conditional compiling by macros.

### Add New Function Module <a name="add_new_module"></a><a href="#contents"><font size=4>(content)</font></a>

1. add a new directory in the `src` directory, with both `*.cpp` and `*.h` files: follow the unit test design,
   you can add a new class or a new function module.

2. add the new directory name into the dependencies of the makefile. (UNFINISHED! by myself)

3. call the new functions/modules/classes in the unit test wrapper codes, and check its correctness.

4. use it in other codes, with cross-module unit test.

5. include the new `*.cpp` file in the `src/C_wrapper/galotfa.h` file, in the end part pf the file that is
   enclosed by the `#ifdef header_only ... #else ... #endif` statement, so that the new functions can work in the header-only case.

6. add a C wrapper function for the new function if you want make it become a public API.

### Codes Structure <a name="codes_structure"></a><a href="#contents"><font size=4>(content)</font></a>

##### `src/tools`

- `prompt.h`: define some macros for prompt message.
  Note: to used the following macro in MPI mode, you need to include the `mpi.h` before `prompt.h`, otherwise CPP can not
  distinguish the `MPI` environment.

  - `println`: print a message with a new line, this is the most commonly used macro and will only print
    the message in the root process if the program is running in MPI mode.
  - `WARN`: print a warning message, based on the `println` macro.
  - `ERROR`: print a error message, based on the `println` macro.

##### `src/parameter`

Note: the section name is case sensitive, but the key/value name is case insensitive.
The `ini_parser` class will parse the ini parameter file into a nested hash table:

- First level: the section name, and the pointer to the second level hash table.
- Second level: the key-value pairs.
- `ini-parser`: a simple ini parameter file parser, which is used to parse the parameter file.

  - available boolean: case insensitive `true` and `false`, `1` and `0`, `on` and `off`, `enable` and `disable`, `yes` and `no`.
  - available value type: number (doesn't distinguish integer and float), string, boolean.
  - comment prefix: `#` and `;`.

- most important methods:
  - `trim`: remove the white spaces and comments after the main content of a line.
  - `split`: split the values into a vector of sub-strings.
  - `line_parser`: get the type of a line, namely a section header or a key-value pair, based on `trim` and `split`.
  - `read`: the main interface used to read the parameter file, based on `line_parser`.
