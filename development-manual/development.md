This documentation is for the code development of the project. I write this documentation for the
developers who are interested in the project. And I wish the documentation can cover all necessary
information for modification of the codes and further development.

Please feel free to raise an issue or give your valuable improvement suggestions for the project
and the documentations.

## Content <a name="content"></a>

1. <a href="#deps">Dependencies</a>
2. <a href="#files">Project Frame</a>
3. <a href="#unit_test">Unit Test</a>
4. <a href="#add_new_module">Add New Function Module</a>
5. <a href="#codes_structure">Codes Structure</a>

## Dependencies <a name="deps"></a><a href="#contents"><font size=4>(content)</font></a>

This project depends on the following tools or libraries:

- [gnu make](https://www.gnu.org/software/make/): the main build tool of the project.
- a c++ compiler which supports C++11: recommended ones are g++>=4.8.5 or clang++>=3.4, or icpx
  (the intel llvm-based compiler).
- [gsl](https://www.gnu.org/software/gsl/): the GNU Scientific Library.
- [hdf5](https://www.hdfgroup.org/solutions/hdf5/): the HDF5 library, which is used to store the analysis results.
- a `MPI` library: `mpich`, `openmpi` or `intel mpi` etc.

## Project Frame <a name="files"></a><a href="#contents"><font size=4>(content)</font></a>

- `Makefile`: the main makefile of the project, with some sub-makefiles in the `make-config` directory.
  - Note: due to the sub-makefiles, the `make` command should always be run in the root directory of the project.
- `src`: the source code directory, each sub-directory is a module (just a logic unit, not the c++20 module)
  of the project.

  - `C_wrapper`: the directory include the public C wrapper APIs of the project.
  - `unit_test`: the directory of unit test wrapper codes, more details in the <a href="#unit_test">Unit Test</a> section.
  - `analysis`: the directory of the core analysis codes.
  - `tools`: some utility functions and classes.
  - `output`: functions for analysis results output.
  - `parameter`: parameter file reading and parsing.
  - `engine`: the main virtual analysis engine, which is a wrapper of other modules.
  - `simcodes`: the interface with some simulation software, at now `Gadget4`, `AREPO`, `GIZMO` and `RAMSES` are supported.

- `documentation`: the directory of the developers' documentation and the details of the analysis codes.
- `test_dir`(optional): the temporary directory for the building and running unit test, which will
  be created by the `make test/mpi_test` command.
- `build`(optional): the temporary directory for the building of the project, which will be created by the
  `make` command.

## Unit Test <a name="unit_test"></a><a href="#contents"><font size=4>(content)</font></a>

### 1. Run test in one command

Run `make test` or `make mpi_test` in the root directory of the project, you can test all unit tests.

The `test` target is for the unit test of the serial case, and the `mpi_test` for parallel case. It will
compile the code and run the unit test.

### 2. Specify test units

By modifying the default value of the `TEST_TARGET` in the Makefile, or run `make test/mpi_test units=<unit to be tested>`
you can specify the which unit to be tested.

### 3. Unit test convention

`galotfa` unit test convention is similar to reference to the practice in `RUST`: They
are all implemented in corresponding modules enclosed by some macros for conditional compiling (e.g.
`debug_parameter` to open the unit test for the parameter module). Such macro will be called "debug macro"
in this documentation. The unit test functions should return 0 for success or 1 for failure.
For unit test of a class, the unit test functions should be public member function `test_xxx` in such class,
where `xxx` is the name of the test case. For unit test of other functions, the unit test functions
should be defined in a `unit_test` namespace that enclosed by the corresponding macros.
This convention is aimed to make the unit test more readable.

The macro of the unit test is defined by `make` in the `Makefile` according to the `TEST_TARGET` variable.

### 4. Add new unit test

There are 5 steps to add a new unit test:

1. consider a corresponding debug macro for a new module part, or choose a existing module and its debug macro
   to control the conditional compilation of the new test. Such macro will be automatically defined `make`
   when run `make test/mpi_test` if you add such macro into the `TEST_TARGET` in the Makefile,
   for example: if you set `TEST_TARGET = <somthing>`, then the `debug_<somthing>` macro will be add
   to `CXXFLAGS` by `-Ddebug_<somthing>`.

2. design the unit test functions, follow the unit test convention.

3. For a new module, add a new `c++` file in the `src/test` directory, and define a wrapper function
   in which you call the test functions defined in the previous step.

4. For a new module, in the `main` function of the `src/unit_test/test.cpp` file, include the new `c++`,
   file create in the last step.

5. Call the test wrapper function defined in the step 3, or the test wrapper the debug macro you choose
   in the step 1, in the `main` function of the `src/unit_test/test.cpp` file.

6. run `make test` or `make mpi_test` to compile and run the test.

## Add New Function Module <a name="add_new_module"></a><a href="#contents"><font size=4>(content)</font></a>

1. add a new directory in the `src` directory, with both `*.cpp` and `*.h` files: follow the unit test design,
   you can add a new class or a new function module.

2. add the new directory name into the dependencies of the makefile. (UNFINISHED! by myself)

3. call the new functions/modules/classes in the unit test wrapper codes, and check its correctness.

4. use it in other codes, with cross-module unit test.

5. include the new `*.cpp` file in the `src/C_wrapper/galotfa.h` file, in the end part pf the file that is
   enclosed by the `#ifdef header_only ... #else ... #endif` statement, so that the new functions can work in the header-only case.

6. add a C wrapper function for the new function if you want make it become a public API.

## Codes Structure <a name="codes_structure"></a><a href="#contents"><font size=4>(content)</font></a>

### `src/tools`

- `prompt.h`: define some macros for prompt message.
  Note: to used the following macro in MPI mode, you need to include the `mpi.h` before `prompt.h`, otherwise CPP can not
  distinguish the `MPI` environment.

  - `println`: print a message with a new line, this is the most commonly used macro and will only print
    the message in the root process if the program is running in MPI mode.
  - `fprintln`: similar but for `fprintf`.
    the message in the root process if the program is running in MPI mode.
  - `WARN`: print a warning message, based on the `fprintln` macro.
  - `ERROR`: print a error message, based on the `fprintln` macro.

- `prompt.cpp`: the unit test wrapper for the prompt module.

### `src/parameter`

Note: the section name is case sensitive, but the key/value name is case insensitive.
The `ini_parser` class will parse the ini parameter file into a hash table.

- `ini-parser`: a simple ini parameter file parser, which is used to parse the parameter file.

  - allowed boolean value in the ini file: case insensitive `true` and `false`, `on` and `off`, `enable` and `disable`, `yes` and `no`.
  - available value type: number (doesn't distinguish integer and float), string, boolean.
  - comment prefix: `#` and `;`, string after the prefix will be treated as comments.

- most important methods:
  - `trim`: remove the white spaces and comments after the main content of a line.
  - `split`: split the values into a vector of sub-strings.
  - `line_parser`: get the type of a line, namely a section header or a key-value pair, based on `trim` and `split`.
    If the line is in boolean type, its content will set as "true" for better consistence of internal usage.
  - `read`: the main interface used to read the parameter file, based on `line_parser`.
  - `insert_to_table`: insert the parsed key-value pair into the hash table. The hash table is defined with
    string key and a structure value (`galotfa::ini::Value`) pair. The key of the hash table = section name +
    "_" + key name of parameter in the ini file, where the space in the section name will be replaced by `_`.
  - `get_xxx`: the function to extract the value of a key in the parameter file. Support to get boolean, number, string,
    and vector of number and string.
  - `has`: check whether a key exist in the parameter file.

### `src/output`

#### Convention of the data output

- `galotfa` use `hdf5` for data output, where different analysis modules will be stored in different files.
- For better performance, `galotfa` will store the analysis results with chunked dataset to save space.
- `galotfa` will use a virtual stack in the main process to store the analysis results, and output the data
  to the hdf5 file when the stack is full or the simulation is finished. This strategy is due to the
  uncertainty of how many synchronized time steps will be analyzed during a simulation.
  This can avoid the frequent opening and closing of the hdf5 file, which is time consuming.
- It seems to be possible to use the `MPI` parallel hdf5 IO to improve the performance, but there is few
  documentations about this feature, and such feature is not always activated in the hdf5 library, so
  `galotfa` only collect the analysis results into the main process and output the data in it.

#### Convention for array

To simplify the functions parameters and better performance, `galotfa` always use 1D C-style array
to store the analysis results. When store a multi-dimensional array, the array will be stored as different
block in such 1D array, follow the row major convention. For example, a 2D array `a[i][j]` will be stored as `a[i*N+j]`,
where `N` is the length of the second dimension of the array. The analogy is also true for higher
dimensional array, e.g. `i*N + j*M + k`, where the block size multiplied by the index is the size of
the sub-space of the array.
