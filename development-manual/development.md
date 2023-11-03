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

`galotfa` will enable the `-fsanitize=address,leak,undefined` check in the debug mode, which will check the
address leak and undefined behavior of the codes. Such check will slow down the codes, and may cause
some tedious error. You can remove such flags if you want, which is located in `make-config/flags` file.

## Project Frame <a name="files"></a><a href="#contents"><font size=4>(content)</font></a>

- `Makefile`: the main makefile of the project, with some sub-makefiles in the `make-config` directory.
  - Note: due to the sub-makefiles, the `make` command should always be run in the root directory of the project.
- `src`: the source code directory, each sub-directory is a module / modules (just a logic unit, not the
  C++20 module) of the project.
  - `galotfa.h`: the C wrapper header file, which is used to provide a public API for the project.
  - `unit_test`: the directory of unit test wrapper codes, more details in the <a href="#unit_test">Unit Test</a>
    section.
  - `analysis`: the directory of the core analysis codes.
  - `tools`: some utility functions and classes.
  - `output`: functions for analysis results output.
  - `parameter`: parameter file reading and parsing.
  - `engine`: the main virtual analysis engine, which is a wrapper of other modules.
  - `simcodes`: the interface with some simulation software, at now `Gadget4`, `AREPO`, `GIZMO` and `RAMSES`
    are supported.
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

### List of modules <a name="list_of_modules"></a>

- <a href="#src_engine">`src/engine`</a>
- <a href="#src_tools">`src/tools`</a>
- <a href="#src_parameter">`src/parameter`</a>
- <a href="#src_output">`src/output`</a>

### `src/engine`: <a id="src_engine"></a><a href="#list_of_modules"><font size=4>(src list)</font></a>

This part organize the other modules to work together.

#### Design philosophy

The so-called virtual analysis engine is designed to be a one-time calculator at required synchronized time steps.

In a black box view: the virtual analysis engine is a calculator associated with a monitor, which will do
all calculation by itself. All you need to do is to press some buttons (public APIs) on the monitor, then what
you experienced is just like you are using a calculator. Namely, press `start` button (the constructor) to start
the engine, press `run_with` button to run the engine process your data every time you get some data
(from the simulation), and finally press `stop` button (the destructor) to stop the engine, and the machine
will save the results for you. There are mainly 4 parts of it, a design scheme of the engine (the ini
parameter file), a printer (writer with hdf5), a calculator and a monitor.
For the programmer, the so called virtual analysis engine is the 4 parts, but for the user, the virtual analysis
engine is just the monitor.

In a technical point of view: The virtual analysis engine is a higher level wrapper of other modules,
which will combine other modules together to finish the on-the-fly analysis.

#### Public APIs

- `monitor.run_with(...)`: the main open API of the monitor class, which can be imagined as a monitor screen
  of a analysis engine. This API with receive the data from the simulation and call the other APIs to
  finish the on-the-fly analysis.
- `calculator.run_once`: the API to call the calculator to analysis of the data at one synchronized time step.
  This function is a wrapper of the previous APIs.
- `calculator.feedback()`: the wrapper to return a vector of the analysis results' pointers to the monitor,
  which will be used to write the analysis results into the hdf5 file.
- `calculator.recv_data(...)`: the API between the companion of the `monitor.push_data(...)` that receive the data
  from the monitor and the real analysis parts.
- `calculator.is_active()`: return the value of the status flag, which indicate whether the calculator is
  active or not.

#### Other important APIs

- `monitor.init()`: initialize the monitor, which will create the output directory if it does not exist,
  then create the writer objects based on the parameter file. This function is aimed to make the constructor
  of the monitor class more readable.
- transfer the data from the simulation to the virtual analysis engine.
- `monitor.push_data(...)`: the API between the monitor of the analysis engine, which push the data and the
  parameter to the virtual analysis engine.
- `monitor.save()`: the API between the monitor and the writer class, to save the analysis results at each step into
  hdf5 files.
- `calculator.start()`: start up the analysis engine.
- `calculator.stop()`: stop the analysis engine, which will free some status flags and resources.
- `calculator.pre_process(...)`: the wrapper of the pre-process part of the analysis engine, which will be
  called before the main analysis function in each synchronized time step.
- `calculator.model()`: the wrapper of the model level analysis of the analysis engine.
- `calculator.particle()`: the wrapper of the particle level analysis part of the analysis engine.
- `calculator.orbit_curve()`: the wrapper of the orbit level log part of the analysis engine.
- `calculator.group()`: the wrapper of the group level analysis part of the analysis engine.
- `calculator.post()`: the wrapper of the post process part of the analysis engine, which will
  be called after all synchronized time steps.

#### Implementation details

- class `monitor`: the virtual analysis engine's monitor.

  - `init()`: create the output directory and open the hdf5 files, only to make the constructor more readable.
  - `run_with(...)`: arguments are pointers of the simulation data, which must includes array of particle id (1D),
    mass (1D), particle type (1D), coordinate (2D), velocity (2D) of particles, and a time stamp variable, a
    integer to specify the length of the arrays. There is additional argument for the potential tracers'
    particle type , if the codes support potential tracer feature (see in other section ???).
  - `galotfa::parameter::para* para`: the pointer to the parameter class, will be initialized in the constructor,
    based on a temporary `ini_parser` object. The default ini parameter filename is specified at here, which
    is `galotfa.ini` in the working directory.
  - `create_writers()`: create the writer objects based on the parameter file, due to the file lock, this
    function should be called only in the main process.

- class `calculator`: the virtual analysis engine's calculator.

  - constructor: with one argument `galotfa::parameter::para&`, which is a reference to the parameter class.
  - some container of the analysis results, which will be used to restore the results from different analysis modules.
  - `recv_data(...)`: the API between the companion of the `monitor.push_data(...)` that receive the data from
    the monitor and deliver the real analysis parts.
  - There are two versions of this function, one of which support the potential tracer.
  - At the end of the function, the `run_once` function will be called to analysis the data at one synchronized
    time step.
  - Actually, the `monitor.push_data(...)` function is an inline function that just call this function,
    to define them as separate functions is just for better readability of the codes.
  - `start()`: start up the analysis engine, which will allocate the memory for the analysis results and set
    some status flags.
  - `stop()`: stop the analysis engine, which will free some status flags and resources.
  - the analysis wrappers (list in the API section): use the parameters to call the pure functions, which
    is implemented in the real analysis modules, to finish the on-the-fly analysis, and then update the
    analysis results that returned by the pure functions.
  - `run_once(...)`: the API to call the calculator to analysis of the data at one synchronized time step.
    This function is a wrapper of the previous APIs and will be automatically called by the `recv_data` function.
  - `feedback(...)`: return the analysis results' pointers to the monitor, due to the limitation of static
    type, the pointers will be stored in a vector of `void*` type, which will be converted to the real type
    in the monitor. Keep this in mind when you want to add a new analysis module, and be careful to use the
    pointers in the vector.

    <font color=red>Note</font>: such pointers' data will be overwritten in the each synchronized time step, so the
    monitor should save the data immediately after the `feedback` function is called.

### `src/tools` <a id="src_tools"></a> <a href="#list_of_modules"><font size=4>(src list)</font></a>

#### Public APIs

For `prompt.h`, `prompt.cpp`:

- `println`: macro, print a message with a next line symbol, which will only print the message in the
  root process if the program is running in MPI mode.
- `fprintln`: macro, similar as `println` but aims to simplify `fprintf`.
- `INFO`: print a information message, based on the `println` macro.
- `WARN`: print a warning message to `stderr`, based on the `fprintln` macro.
- `ERROR`: print a error message to `stderr`, based on the `fprintln` macro.

For `string.h`, `string.cpp`:

- `std::string trim(std::string str, std::string blank)`: remove the leading and trailing blank characters
  in the string `str`. The default blank characters are `" \t\n\r\f\v"`.
- `std::vector<std::string> split(std::string str, std::string delim)`: split the string `str` into a
  vector of strings based on the delimiters in `delim`. The default delimiter is `" "`.
- `std::string replace(std::string str, std::string old_str, std::string new_str)`: replace the `old_str` in the
  string `str` with the `new_str`.

#### Other important APIs

For `prompt.h`, `prompt.cpp`:

<font color=red>Note: to used the following macro in MPI mode, you need to include the `mpi.h` before
include `prompt.h`, otherwise `CPP` can not distinguish the `MPI` environment.</font>

- `CHECK_RETURN(status_flag)`: check the status flag of a boolean value, return 0 for success and 1 for failure.
  If it's failure, print a warning message to the line, file and function where the macro is called.
  This is designed to be used in the unit test functions (the true test function, not the wrappers) to
  check and return the test result.
- `COUNT(<call a unit test function>)`: the function to check and count the result of a unit test, seen in the
  unit test section.
- `SUMMARY(<c str of a module name>)`: should be called after all `COUNT(<somthing>)` statement, which will
  print the summary of the unit test results with the given module name.

#### Implementation details

- `prompt.h`: define some macros for prompt message.

  - `println`: if include `mpi.h`, get the rank of current process, then print the message with a next line
    symbol in the root process. If not include `mpi.h`, just print the message with a next line symbol,
    namely `printf(...); printf("\n");`.
  - `fprintln`: similar as `println` but print to a FILE pointer.
  - `WARN`: print a warning message, based on the `fprintln` macro.
  - `ERROR`: print a error message, based on the `fprintln` macro.
  - `COUNT(<call a unit test function>)`: the function to check the result of a unit test, then increase
    the int variable `success`, `fail` and `unknown` correspondingly, in the local environment where the
    macro is called. This macro should be used only in the unit test wrapper functions, where the unit
    test functions will return 0 for success, 1 for failure and other value for unknown.
  - `SUMMARY(<c str of a module name)`: sum the `success`, `fail` and `unknown` variables, then print the results.
    This macro should be used only in the unit test wrapper functions, after all `COUNT(<somthing>)` statement.

### `src/parameter` <a id="src_parameter"></a> <a href="#list_of_modules"><font size=4>(src list)</font></a>

#### Public APIs

For the `para` struct, which is based on the `ini_parser` class:

- `para::para(ini_parser&)`: the structure of the parameters, which should be initialized with a reference to
  an instance of the `ini_parser` class.
- `para::check()`: check the dependencies and conflicts between the parameters before use them.

For the `ini_parser` class:

- `ini_parser(std::string path_to_file)`: the constructor of the class, which will read the parameter
  file with the given path.
- `ini_parser.get_xxx()`: the function to extract the value of a key in the parameter file. Support
  to get boolean, int, double, string, and vector of int, double and string.
- `ini_parser.has()`: check whether a key exist in the parameter file.

#### Other important APIs

- struct `para`: the structure to store the value of parameters, and read the parameter file based on the `ini_parser` class.
  - call `para.<sec>_<para>` to use the parameter, where `<sec>` is an alias the section name and `<para>` is the
    parameter name. Alias: `glb` for the global section, `pre` for the pre-process section, `md` for the model section,
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
  - member prefix indicates their section in the parameter file: `glb` for the global section, `pre` for the
    pre-process section, `md` for the model section, `ptc` for the particle section, `orb` for the orbit section,
    `grp` for the group section and `post` for the post-process section.
  - constructor: with a reference to a created `ini_parser` object, then update the value of the parameters
    based on the parameter file (with hard code, the ugly but fast way).
  - `check()`: check whether there are some conflicts between the parameters, and whether there are some
    parameters that are lack of. Raise a warning message for each conflict or lack of parameters.

#### Steps to add new parameter into the code

1. Edit the parameter table and explanation in the `README.md` file, during which choose the name, type and
   default value of the new parameter.
2. Edit the `para` class in the `src/parameter/default.h` file, add the parameter into the chosen section
   and default value.
3. Add a line in the update function (`para::para(...)`) to update the value of the new parameter from the ini file.
4. Add a check statement into the check function of `para`(`check()`), to check the possible conflicts and
   lack of the new parameter.
5. Use the new parameter in the analysis code.

### `src/output`: <a id="src_output"></a> <a href="#list_of_modules"><font size=4>(src list)</font></a>

#### Public APIs

- constructor: `writer(std::string)`, create a writer object with the given path to the output file.
  If there is a file with the same name, the writer will create a new file with a `-n` suffix that start from 1,
  where `n` is the smallest integer that make the new file name not exist.
- `writer.create_group(std::string)`: create a hdf5 group with the given name, can create group hierarchy e.g.
  if given a argument like `group1/group2/group3`, the writer will create `group1` , `group2` and `group3`
  recursively if they do not exist. The group name without a prefix `/` will be treated as a root group.
- `writer.create_dataset(std::string dataset, hdf5::info)` or `create_dataset(std::string group, std::string datasetm info)`:
  create a hdf5 dataset with the given name, if the group before the final dataset name, take a example:
  `writer.create_dateset("a/b/c/dataset_name", info)` == `create_group("a/b/c")` then `create_dataset("a/b/c", "dataset_name")`.
- `writer.push(void* ptr, std::string dataset)`: push a array of data to a existing dataset.

#### Other important APIs

- close the hdf5 file: `~writer()`, the destructor will close all resources of the corresponding hdf5 file,
  and you have no need to call a close function manually.
  hdf5 objects, such as dataset and group.

#### Usage

Steps to use the `writer` class to organize the hdf5 file:

1. create a writer object with the given path to the output file: `writer(std::string)`.
2. create an `galotfa::hdf5::info` object that specify the info of the dataset: data type, rank and dimension.
3. create a dataset with the given name and info: `writer::create_dataset(std::string dataset, hdf5::info)`
4. push the data array to the dataset when needed: `writer::push(void* ptr, std::string dataset)`.
5. After all steps, the `writer` will organize the resources of the hdf5 file automatically, there is no need
   to close anything manually.

- <font color=red>NOTE</font>:

  1. The pushed data array should follow the 1D array convention (see <a href="#convention of data array">Convention for arrays of data</a> )
  2. Make sure that the size of the pushed data array is the same as the size of the dataset otherwise the
     behavior is undefined.
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
- For better performance, `galotfa` will store the analysis results with chunked dataset and compress them
  to save space.
- `galotfa` will use a virtual stack in the main process to store the analysis results, and output the data
  to the hdf5 file when the stack is full or the simulation is finished. This strategy is due to the
  uncertainty of how many synchronized time steps will be analyzed during a simulation.
  This can avoid the frequent opening and closing of the hdf5 file, which is time consuming.
- The size of the virtual stack is in units of synchronized time steps in simulation, which is defined as
  a constant `VIRTUAL_STACK_SIZE` in `src/output/writer.h`.
- It seems to be possible to use the parallel hdf5 IO to improve the performance, but there is few
  documentations about this feature, and such feature is not always activated in the hdf5 library, so
  `galotfa` only use a serial mode to write date, which is collected from all processes into the main process.

#### Convention for arrays of data

To simplify the functions parameters and better performance, `galotfa` always use 1D C-style array
to store the analysis results. When store a multi-dimensional array, the array will be stored as different
block in such 1D array, follow the row major convention. For example, a 2D array `a[i][j]` will be stored
as `a[i*N+j]`, where `N` is the length of the second dimension of the array. The analogy is also true
for higher dimensional array, e.g. `i*N + j*M + k`, where the block size multiplied by the index is the
size of the sub-space of the array.

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
  - `writer::create_file(std::string)`: call `open_file(...)` to open the hdf5 file, and create a file node
    with the given name. This function should
    be called only in the main process, due to the hdf5 file lock.
  - `writer::create_group(std::string)`: create a hdf5 group with the given name, can be used to create nested
    groups recursively, e.g. given `group1/group2/group3`, the writer will create `group1` , `group2` and `group3`
    recursively if they do not exist. The group name without a prefix `/` will be treated as a root group.
  - `writer::create_dataset(std::string, galotfa::hdf5::size_info&)`: create a hdf5 dataset with the given
    name, if the string before the final dataset name, take a example: `a/b/c/dataset_name`, will be created
    as group, `a/b/c` here.
  - Note: The previous three create function will return 1 if there is some warning, and 0 for success. So never
    ignore the return value of these functions.
  - `galota::hdf5::node create_datanode(node& parent, std::string& dataset, galotfa::hdf5::size_info&)`: setup the
    dataset of the parent node with the given name and info. `dataset` string should be the name of dataset
    only, e.g. for `/group1/group2/dataset_name` the given value should be `dataset_name`. The parent node
    should be a group node.
  - `open_file(...)`: open a hdf5 file and return its id, private.
  - `push(void* buffer, unsigned long len, std::string dataset_name)`: a template function, the main interface
    to push a array of data to a existing dataset, the dataset name should be the absolute path of the dataset,
    e.g. `/group1/group2/dataset_name`. If such dataset does not exist, the writer will create it automatically.
    The data type of the dataset will be restored by `node`, during creation of the dataset.
