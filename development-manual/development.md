This documentation is for the ones who are interested in the code development of the project.
It will include the APIs and some Implementation details of the codes, but not for the algorithm details.
For the algorithm details of the analysis, please check the `quantity.md` file (TO-BE FINISHED).
And I wish the documentation can cover all necessary information for modification of the codes
and further development.

Please feel free to raise an issue or give your valuable improvement suggestions for the project
and the documentations.

---

## Content <a name="content"></a>

1. <a href="#deps">Dependencies</a>
2. <a href="#debug">Debug Mode</a>
3. <a href="#files">Project Frame</a>
4. <a href="#unit_test">Unit Test</a>
5. <a href="#add_new_module">Add New Function Module</a>
6. <a href="#codes_structure">Codes Structure</a>

---

## Dependencies <a name="deps"></a><a href="#contents"><font size=4>(content)</font></a>

This project depends on the following tools or libraries:

- [make](https://www.gnu.org/software/make/): the main build tool of the project.
- a c++ compiler with full support of C++11: recommended ones are g++>=4.8.5 or clang++>=3.9.1, or any icpx
  (the intel llvm-based compiler).
- For the default Makefile of the project, the priority of the compilers: clang++ > icpx > icpc > g++.
- [gsl](https://www.gnu.org/software/gsl/): the GNU Scientific Library.
- [hdf5](https://www.hdfgroup.org/solutions/hdf5/): the HDF5 library, which is used to store the analysis results.
- a `MPI` library: `mpich`, `openmpi` or `intel mpi` etc.

---

## Debug Mode<a name="debug"></a><a href="#contents"><font size=4>(content)</font></a>

`galotfa` will enable the `-fsanitize=address,leak,undefined` check in the debug mode, which will check the address
leak and undefined behavior of the codes. Such check will slow down the codes, and may cause some tedious error.
You can remove such flags if you want, which is located in `make-config/flags` file.

## Project Frame <a name="files"></a><a href="#contents"><font size=4>(content)</font></a>

- `Makefile`: the main makefile of the project, with some sub-makefiles in the `make-config` directory.
  - Note: due to the sub-makefiles, the `make` command should always be run in the root directory of the project.
- `src`: the source code directory, each sub-directory is a module / modules (just a logic unit, not the c++20 module)
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

---

## Unit Test <a name="unit_test"></a><a href="#contents"><font size=4>(content)</font></a>

### 1. Run test

In root directory of the project, run: `make test/mpi_test mode=debug/release units=<unit to be tested>`.

If the `mode` or `units` are not specified, the default value will be used, which are `mode=release`
and `units=TEST_TARGET`, `TEST_TARGET` is set in the `Makefile`.

### 2. Unit test convention

- `galotfa` unit test convention is inspired by `RUST` built-in unit test style: all unit test codes
  are all implemented in corresponding modules enclosed by some macros for conditional compiling (e.g.
  `debug_parameter`). Such macro will be called "debug macro" in this documentation.
- For class, the unit test functions should be public member function `test_xxx` in such class enclosed by
  debug macro, where `xxx` is the name of the test case.
- For non-class functions, the unit test functions should be defined in a `unit_test` namespace that enclosed by
  the corresponding debug macro.
- The return code of a unit test functions in each module: 0 for success, 1 for failure and other value
  for unknown status. In general, the `unknown` status means that the unit test has not covered all possibilities.
- The debug macro of the unit test is defined by `make` in the `Makefile` according to the `TEST_TARGET`
  variable or the run time argument `units` of the `make test/mpi_test` command.
- There are wrappers for each module, in which the unit test functions are called and the test results are
  counted. The wrappers are defined in the `src/unit_test/` directory.
- As available in the existing test wrappers, there are three integer to count the test results: `success`,
  `fail` and `unknown`, to call the unit test function and count its result, just need to use the
  `COUNT(<call a unit test function>)` macro.
- For unit test functions, to get the test result, just need to use the `CHECK_RETURN(<boolean value>)` macro,
  which will return 0 for success and 1 for failure for some boolean value related to the test results, for example:
  `success = (test_result1 == expected_result1) && (test_result2 == expected_result2); CHECK_RETURN(success)`.

### 3. Add new unit test

If you want to add a new unit test, please check the convention in the last section first.

There are 5 steps to add a new unit test:

1. Choose a in-used debug macro or define a new debug macro to control the unit test, the in-used debug
   macros can be found in the `src/unit_test/test.cpp` file.
2. design the unit test functions of a module, follow the unit test convention.
3. For a new module, add a new `c++` file in the `src/test` directory, and define a wrapper function in it
   to call the new unit test functions defined in the step 2. Then include the new `c++` file in the
   `src/unit_test/test.cpp` file analogous to the other modules. You can design your own style to print the
   test results in the end of the unit test wrapper function, or just use the existing style:

   - define integer variables `success = 0`, `fail = 0` and `unknown = 0` at the beginning of the wrapper
     function.
   - call `COUNT(<call a unit test function>)` for each unit test function.
   - call `SUMMARY(<c str of a module name>)` in the end of the wrapper function, which will print the
     summary of the unit test results with the given module name.

4. For a existing module, call the new unit test functions defined in the step 2 in the module's `c++` file
   which defines a wrapper function for such module. You can follow the unit test convention and existing
   wrappers to do this:

   - define integer variables `success = 0`, `fail = 0` and `unknown = 0` at the beginning of the wrapper
     function.
   - call `COUNT(<call a unit test function>)` for each unit test function.
   - call `SUMMARY(<c str of a module name>)` in the end of the wrapper function, which will print the
     summary of the unit test results with the given module name.

5. run `make test` or `make mpi_test` to compile and run the test.

---

## Add New Function Module <a name="add_new_module"></a><a href="#contents"><font size=4>(content)</font></a>

1. add a new directory in the `src` directory, with both `*.cpp` and `*.h` files: follow the unit test design,
   you can add a new class or a new function module.
2. add the new directory name into the dependencies of the makefile. (UNFINISHED! by myself)
3. call the new functions/modules/classes in the unit test wrapper codes, and check its correctness.
4. use it in other codes, with cross-module unit test.
5. include the new `*.cpp` file in the `src/C_wrapper/galotfa.h` file, in the end part pf the file that is
   enclosed by the `#ifdef header_only ... #else ... #endif` statement, so that the new functions can work in the header-only case.
6. add a C wrapper function for the new function if you want make it become a public API.

---

## Codes Structure <a name="codes_structure"></a><a href="#contents"><font size=4>(content)</font></a>

### `src/tools`

#### APIs

For `prompt.h`, `prompt.cpp`:

<font color=red>Note: to used the following macro in MPI mode, you need to include the `mpi.h` before include `prompt.h`,
otherwise `CPP` can not distinguish the `MPI` environment.</font>

- `println`: macro, print a message with a next line symbol, which will only print the message in the
  root process if the program is running in MPI mode.
- `fprintln`: macro, similar as `println` but aims to simplify `fprintf`.
- `WARN`: print a warning message to `stderr`, based on the `fprintln` macro.
- `ERROR`: print a error message to `stderr`, based on the `fprintln` macro.
- `CHECK_RETURN(status_flag)`: check the status flag of a boolean value, return 0 for success and 1 for failure.
  If it's failure, print a warning message to the line, file and function where the macro is called.
  This is designed to be used in the unit test functions (the true test function, not the wrappers) to
  check and return the test result.
- `COUNT(<call a unit test function>)`: the function to check and count the result of a unit test, seen in the
  unit test section.
- `SUMMARY(<c str of a module name>)`: should be called after all `COUNT(<somthing>)` statement, which will
  print the summary of the unit test results with the given module name.

For `string.h`, `string.cpp`:

- `std::string trim(std::string str, std::string blank)`: remove the leading and trailing blank characters in the
  string `str`. The default blank characters are `" \t\n\r\f\v"`.
- `std::vector<std::string> split(std::string str, std::string delim)`: split the string `str` into a vector of strings
  based on the delimiters in `delim`. The default delimiter is `" "`.
- `std::string replace(std::string str, std::string old_str, std::string new_str)`: replace the `old_str` in the
  string `str` with the `new_str`.

#### Implementation details

- `prompt.h`: define some macros for prompt message.

  - `println`: if include `mpi.h`, get the rank of current process, then print the message with a next line
    symbol in the root process. If not include `mpi.h`, just print the message with a next line symbol,
    namely `printf(...); printf("\n");`.
  - `fprintln`: similar as `println` but print to a FILE pointer.
  - `WARN`: print a warning message, based on the `fprintln` macro.
  - `ERROR`: print a error message, based on the `fprintln` macro.
  - `COUNT(<call a unit test function>)`: the function to check the result of a unit test, then increase
    the int variable `success`, `fail` and `unknown` correspondingly, in the local environment where the macro is called.
    This macro should be used only in the unit test wrapper functions, where the unit test functions will
    return 0 for success, 1 for failure and other value for unknown.
  - `SUMMARY(<c str of a module name)`: sum the `success`, `fail` and `unknown` variables, then print the results.
    This macro should be used only in the unit test wrapper functions, after all `COUNT(<somthing>)` statement.

### `src/parameter`

#### APIs

- `ini_parser(std::string path_to_file)`: the constructor of the class, which will read the parameter file with the given path.
- `ini_parser.get_xxx()`: the function to extract the value of a key in the parameter file. Support to get boolean, int,
  double, string, and vector of int, double and string.
- `ini_parser.has()`: check whether a key exist in the parameter file.
- structure `para`: the structure to store the value of parameters, and read the parameter file based on the `ini_parser` class.
  - call `para.<sec>_<para>` to use the parameter, where `<sec>` is an alias the section name and `<para>` is the
    parameter name. Alias: `gb` for the global section, `pre` for the pre-process section, `md` for the model section,
    `ptc` for the particle section, `orb` for the orbit section, `grp` for the group section and `post` for the
    post-process section.

#### Implementation details

Note: the section name is case sensitive, but the key/value name is case insensitive.
The `ini_parser` class will parse the ini parameter file into a hash table.

- namespace `ini`:

  - enum class `LineType`: the type of a line in the ini file, `section`, `key_value` or `empty`.
  - enum class `ValueType`: the type of a key-value pair, `boolean`, `number(s)` or `string(s)`.
  - structure `Line`: the structure to store the content of a line in the ini file.
  - structure `Value`: similar but for the value of a key-value pair.

- `ini-parser`: a simple ini parameter file parser, which is used to parse the parameter file.

  - allowed boolean value in the ini file: case insensitive `true` and `false`, `on` and `off`, `enable` and `disable`, `yes` and `no`.
  - available value type: number (doesn't distinguish integer and float), string, boolean.
  - comment prefix: `#` and `;`, string after the prefix will be treated as comments.

- most important methods:

  - `trim`: a wrapper of `galotfa::string::trim`, which will further remove the comment part of the string.
  - `split`: a wrapper of `galotfa::string::split`.
  - `line_parser`: get the type of a line, namely a section header or a key-value pair, based on `trim` and `split`.
    If the line is in boolean type, its content will set as "true" for better consistence of internal usage.
  - `read`: the main interface used to read the parameter file, based on `line_parser`.
  - `insert_to_table`: insert the parsed key-value pair into the hash table. The hash table is defined with
    string-type key and a `ini::Value`-type value. The key of the hash table = section name +
    "\_" + key name of parameter in the ini file, where the space in the section name will be replaced by `_`.

- class `para`: define the default value of some parameters, and update the value according the ini parameter file.
  - member prefix indicates their section in the parameter file: `gb` for the global section, `pre` for the
    pre-process section, `md` for the model section, `ptc` for the particle section, `orb` for the orbit section,
    `grp` for the group section and `post` for the post-process section.
  - constructor: with a reference to a created `ini_parser` object, then update the value of the parameters
    based on the parameter file (with hard code, the ugly but fast way).

#### Steps to add new parameter into the code

1. Edit the parameter table and explanation in the `README.md` file, during which choose the name, type and
   default value of the new parameter.
2. Edit the `para` class in the `src/parameter/default.h` file, add the parameter into the chosen section
   and default value.
3. Add a line in the update function (???) to update the value of the new parameter from the ini file.
4. Use the new parameter in the analysis code.

### `src/output`: the hdf5 `writer` class

#### APIs

- constructor: `writer(std::string)`, create a writer object with the given path to the output file.
  If there is a file with the same name, the writer will create a new file with a `-n` suffix that start from 1,
  where `n` is the smallest integer that make the new file name not exist.
- close the hdf5 file: `~writer()`, the destructor will close all resources of the corresponding hdf5 file,
  and you have no need to call a close function manually.
  hdf5 objects, such as dataset and group.
- `writer.create_group(std::string)`: create a hdf5 group with the given name, can create group hierarchy e.g.
  if given a argument like `group1/group2/group3`, the writer will create `group1` , `group2` and `group3`
  recursively if they do not exist. The group name without a prefix `/` will be treated as a root group.
- `writer.create_dataset(std::string dataset, hdf5::info)` or `create_dataset(std::string group, std::string datasetm info)`:
  create a hdf5 dataset with the given name, if the group before the final dataset name, take a example:
  `writer.create_dateset("a/b/c/dataset_name", info)` == `create_group("a/b/c")` then `create_dataset("a/b/c", "dataset_name")`.
- `writer.push(void* ptr, std::string dataset)`: push a array of data to a existing dataset.
- steps to use the `writer` class to organize the hdf5 file:

  1. create a writer object with the given path to the output file: `writer(std::string)`.
  2. create an `galotfa::hdf5::info` object that specify the info of the dataset: data type, rank and dimension.
  3. create a dataset with the given name and info: `writer::create_dataset(std::string dataset, hdf5::info)`
  4. push the data array to the dataset when needed: `writer::push(void* ptr, std::string dataset)`.
  5. After all steps, the `writer` will organize the resources of the hdf5 file automatically, there is no need to close
     anything manually.

- <font color=red>NOTE</font>:

  1. The pushed data array should follow the 1D array convention (see <a href="#convention of data array">Convention for arrays of data</a> )
  2. Make sure that the size of the pushed data array is the same as the size of the dataset otherwise the behavior is undefined.
     - Although there is a `len` parameter to specify the 1D array size, but the for a C-style array, such
       parameter is only for check. And the code actually can not detect the size of the array. The cost
       of the UB is for better performance with raw C-style array.
  3. The name of group and dataset is case sensitive and unique, there can not be
     dataset or group share the same under the same group, e.g. `a/a/a` is legal for either a dataset `a` or a
     subgroup `a` under parent group `a/a`, but under group `a/a/` there can not be a dataset and a subgroup
     with the same name `a`. This is not a behavior of `galotfa`, but a convention of the hdf5 library.
  4. All APIs are designed to be used in the main process, due to the hdf5 file lock, and the unit test of
     this part is only designed for the serial mode, `make mpi_test` with `units=output` definitely fails.

#### Convention of the data output

- `galotfa` use `hdf5` for data output, where different analysis modules will be stored in different files.
- For better performance, `galotfa` will store the analysis results with chunked dataset and compress them to save space.
- `galotfa` will use a virtual stack in the main process to store the analysis results, and output the data
  to the hdf5 file when the stack is full or the simulation is finished. This strategy is due to the
  uncertainty of how many synchronized time steps will be analyzed during a simulation.
  This can avoid the frequent opening and closing of the hdf5 file, which is time consuming.
- The size of the virtual stack is in units of synchronized time steps in simulation, which is defined as
  a constant `VIRTUAL_STACK_SIZE` in `src/output/writer.h`.
- It seems to be possible to use the parallel hdf5 IO to improve the performance, but there is few
  documentations about this feature, and such feature is not always activated in the hdf5 library, so
  `galotfa` only use a serial mode to write date, which is collected from all processes into the main process.

#### Convention for arrays of data <a name="convention of data array"></a>

To simplify the functions parameters and better performance, `galotfa` always use 1D C-style array
to store the analysis results. When store a multi-dimensional array, the array will be stored as different
block in such 1D array, follow the row major convention. For example, a 2D array `a[i][j]` will be stored as `a[i*N+j]`,
where `N` is the length of the second dimension of the array. The analogy is also true for higher
dimensional array, e.g. `i*N + j*M + k`, where the block size multiplied by the index is the size of
the sub-space of the array.

#### Implementation details

- namespace `galotfa::hdf5`:

  - enum class `NodeType`: the type of a hdf5 node,`file` or `group` or `dataset`, also a `uninitialized` type for
    the default constructor of the `node` class.
  - `info`: the structure to specify the info of a dataset, which include the data type (`.data_type`, hdf5 type id),
    rank (`.rank`, unsigned int) and dimension (`.dims`, `std::vector` of `hsize_t`).
  - class `node`: the class for a hdf5 node, which can be a group or a dataset, this class will take
    over the resources organization of the hdf5 file, so there is no need to close the hdf5 objects manually.
    - constructor: `node(std::string, NodeType)`, create a node object with the given name and type. This
      constructor should be called only for the file node, as its parent node is `nullptr`.
    - constructor: `node(node*, std::string, NodeType)`, create a node object with the given name, type and
      parent node. If all sub-nodes are created by this constructor correctly, the tree structure of the
      nodes is the same as the hdf5 file and can organize their resources correctly.

- `writer(std::string)`: the class for data output, which require a string to specify the path to the output
  file for initialization. This function can only be used in the main process, due to the hdf5 file lock.

  - `nodes`: the hash map with string-type key, and `galotfa::hdf5::node`-type value, the key is the absolute
    path of the node, e.g. `/group1/group2/dataset_name` and `/` for the file.

    - Note: due to there is no default constructor for the `node` class, to call a `node` object in the hash map,
      you need to use the `at` method.
    - The key must start with `/` and end without `/`.
    - (Current feature) The `nodes` only support push nodes and clean the whole tree structure. So the `nodes`
      is cleaned only in the destructor of the writer. (This is due to `nodes` use string-type key and
      there is no any tree structure in the hash map, so it's hard to clean the tree structure of the individual
      node).

  - `writer::~writer()`: the destructor of the class, which will close the hdf5 file and the created hdf5 objects,
    such as dataset and group.

  - `writer::create_file(std::string)`.

  - `writer::create_group(std::string)`: create a hdf5 group with the given name, can be used to create nested
    groups recursively, e.g. given `group1/group2/group3`, the writer will create `group1` , `group2` and `group3`
    recursively if they do not exist. The group name without a prefix `/` will be treated as a root group.

  - `writer::create_dataset(std::string, galotfa::hdf5::size_info&)`: create a hdf5 dataset with the given name, if the string before
    the final dataset name, take a example: `a/b/c/dataset_name`, will be created as group, `a/b/c` here.

  - Note: The previous three create function will return 1 if there is some warning, and 0 for success. So never
    ignore the return value of these functions.

  - `galota::hdf5::node create_datanode(node& parent, std::string& dataset, galotfa::hdf5::size_info&)`: setup the dataset
    of the parent node with the given name and info. `dataset` string should be the name of dataset only, e.g. for
    `/group1/group2/dataset_name` the given value should be `dataset_name`. The parent node should be a group node.

  - `open_file`: open a hdf5 file and return its id, private.

  - `push(void* buffer, unsigned long len, std::string dataset_name)`: the main interface to push a array of data to a existing dataset,
    the dataset name should be the absolute path of the dataset, e.g. `/group1/group2/dataset_name`. If such
    dataset does not exist, the writer will create it automatically. The data type of the dataset will be restored
    by `node`, during creation of the dataset.

### `src/engine`: the virtual analysis engine

This part organize the other modules to work together.

#### APIs

`manager.sim_data()`: the API between the manager of the virtual analysis engine and the simulation, to
transfer the data from the simulation to the virtual analysis engine.
`manager.push_data()`: the API between the manager of the analysis engine, which push the data and the
parameter to the virtual analysis engine.
`manager.collect()`: the API between the manager and the real analysis parts. This function will collect the
analysis results from different analysis modules.
`manager.write()`: the API between the manager and the writer class, to write the analysis results to the hdf5 file.

#### Implementation details

- class `manager`:
  - `galotfa::parameter::para* para`: the pointer to the parameter class, will be initialized in the constructor,
    based on a temporary `ini_parser` object. The default ini parameter filename is specified at here, which
    is `galotfa.ini` in the working directory.
