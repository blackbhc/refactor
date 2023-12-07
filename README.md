`galotfa`: <font size=4>**gal**</font>actic <font size=4>**o**</font>n-<font size=4>**t**</font>he-<font size=4>**f**</font>ly <font size=4>**a**</font>nalysis,
is a library of on-the-fly analysis for astronomical N-body simulations.

---

## <a id="contents">Contents</a>

- <a href="#scheme">Design scheme</a>
- <a href="#feature">Features of `galotfa`</a>
- <a href="#fork_gadget4">Fork of `Gadget4`</a>
- <a href="#install">Installation</a>
- <a href="#usage">Usage</a>

---

## Design scheme <a href="#contents"><font size=4>(contents)</font></a> <a id="scheme"></a>

The on-the-fly analysis is done at specified synchronized time steps. Its workflow is:

!!!!! To be updated!

<a id="workflow"></a>

```mermaid
graph TB
Data((Data))
Pre[Pre-process]
Model[Model level]
Par[Particle level]
Pop[Population level]
Selct[Particle selection]
Snap([Expansion snapshot])
LogMd([Log of model])
LogOC([Log of orbital curves])
LogPop([Log of populations])

Data --> Pre
Pre --> Model
Pre --> Par
Pre --> Selct
Par --> Pop
Model --> Pop
Pop --> LogPop
Par --> Snap
Selct --> LogOC
Model --> LogMd
```

Every box is a separate module in the codes. The details of them are explained in the file
`development-manual/development.md`. The runtime parameters of each module are specified in the
`galotfa.ini` file, which is a INI format file. The parameters are explained in <a href="#usage">Usage</a>.

---

## Features of `galotfa` <a href="#contents"><font size=4>(contents)</font></a> <a id="feature"></a>

1. Out-of-the-box: in general there is no need to modify the simulation code or parameter files. Just run
   simulation with demo ini files is enough. Feel free to raise your issue, if you feel hard to use any part
   of `galotfa`.
2. User friendly program API and usage guidance: most functions of `galotfa` can be used out-of-the-box,
   and we also provide detailed informations for persons who are interested to change the codes.
3. Extensible: easy to add new analysis functions in `galotfa`, and easy use `galotfa` in your own simulation
   code (if such code follows the general simulation convention in the society).
   - The compiled shared library of `galotfa` offers C-style APIs, so it can be in simulation codes written in
     many languages.
   - Although this project is concentrated on disk galaxy simulations, it can also be easily extended to other
     types of simulations, such as cluster simulations, cosmology simulations, etc.
4. You-Complete-Me flavor: if you need more wonderful features, you can make this project better.
5. Least dependency on other libraries: only standard libraries or widely used libraries are used in this project,
   so it's easy to compile and install in most systems.
6. Fast: use `MPI` to parallelize the analysis, and use efficient algorithms to calculate the quantities.
7. All output files are in `HDF5` format, which is a widely used format in astronomy.

---

## Fork of `Gadget4` <a href="#contents"><font size=4>(contents)</font></a> <a id="fork_gadget4"></a>

There is a built-in fork of `Gadget4` in the `galotfa` repo, which is a modified version of
[Springel](https://wwwmpa.mpa-garching.mpg.de/gadget4/)'s famous codes with following additional features:

1. Zero-mass potential tracers: can use a separate particle types to served as potential tracer, which
   has 0 mass. During gravity calculation, their position is fixed to some anchor particles, so they can
   be used to measure the potential at their positions.

   - This feature is controlled by an additional config parameter `ZERO_MASS_POT_TRACER` in the configuration
     file of `Gadget4`.
   - Cost of such tracers: additional memory and time cost of gravity calculation, slightly change the
     gravity tree's structure.
   - The potential at the position of the potential tracers will be output to a separate file, which can be
     used get the potential at time steps that have no snapshot output.

2. Additional runtime parameters related to the potential tracers:

- `PotTracerType`: specify the particle type of potential tracer. Note that this should be consistent
  with the given initial condition, namely it should really be zero-mass particles in the initial condition.
- `PotOutStep`: the output period in unit of synchronized time steps.
- `PotOutFile`: output filename.
- `RecenterPartType`: the particle type(s) used to anchor the positions of the tracers. In some cases, if
  the whole galaxy is drifting and the tracers are fixed then their measured potential may be outside of the
  galaxy. So you can use some particles to anchor the positions of the tracers, then tracers are rigidly attached
  to the anchors' center-of-mass. For example, for disk galaxy, use disk particles to anchor the tracers is a
  good choice.
- `RecenterThreshold`: the threshold to determine whether the calculation of the anchors' center-of-mass is
  convergent or not. The unit is the internal length during the simulation.

3. The config parameter to enable `galotfa` or not: `GALOTFA_ON`.

---

## Installation <a href="#contents"><font size=4>(contents)</font></a> <a id="install"></a>

### Dependencies

First, you need to check the following dependencies before you install `galotfa`:

1. `galotfa` needs the following non-standard libraries for compilation:

- A `MPI` library: `openmpi`, `mpich`, `intel-mpi`, etc.
- `gsl` library.
- `hdf5` library.

2. `make`: the compilation is organized by `make`.

   Run `which make` to check whether you have `make` or not. If you get a path, then you have `make`.
   Otherwise, you don't have `make` and you need to install it first. You can search on the internet
   to find how to install `make` on your system.

3. A `c++` compiler with `c++11` support, e.g. `g++`>4.8.5 or `clang++`>3.9.1 are recommended.

   You can run `g++ --version`, `clang++ --version` or `c++ --version` to check whether you have such compiler
   or not. If you get a version number, then you have such compiler. Otherwise, you need to install
   a suitable compiler first. You can search on the internet to find how to install `g++` on your system.

4. At least one tool to download the `galotfa` repo: `git`, `wget` or `curl`.

### Download and install step by step

1. Clone the `galotfa` repo: run `git clone -b main https://github.com/blackbhc/galotfa --depth=1`

   If you don't have `git`, try run `wget -O- https://github.com/blackbhc/galotfa/archive/main.tar.gz | tar xzv`
   or `curl -O https://github.com/blackbhc/galotfa/archive/main.tar.gz && tar xzv mian.tar.gz`.

2. Run `cd galotfa`

3. run `mkdir -p <path/to/install>`:

   Create the directory for the installation of `galotfa`, if it already exists, you can skip this step.

4. run `make install mode=release type=library prefix=</path/to/install>`:

   Install the `galotfa` to the directory specified by `prefix`, which should be the same as
   the one you specified in the last step.

   - the `mode` option can be `release` or `debug`, default is `release` which has `-O3` optimization mode.
     `debug` make the compiled library includes debug information, which is only useful for developers.
     Note that the debug mode has some platform dependency issues, and may not work in some cases.

   - the `type` option can be `header-only` or `library` (default).

     - `header-only`: only copy the header and source files, and don't compile a library file. Note that
       `galotfa` in this mode can only be used in `c++` simulation codes.
     - `library`: build the shared library, works for any simulation code supporting C-style APIs.

   Note:

   1. `header-only` is recommended for the first time installation. Such option will define a `header-only` macro
      in the `galotfa` header files, which will make the library can be used without linking to the library files.
   2. If you encounter any error during the installation, after you resolve the error, please run `make clean`
      to clean the build files before you try to install again.

5. Configure environment variables: `CPATH`, `LIBRARY_PATH` and `LD_LIBRARY_PATH`

   - (temporary configuration) run `export CPATH=$CPATH:<prefix>/include`, `export LIBRARY_PATH=$LIBRARY_PATH:<prefix>/lib`,
     `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<prefix>/lib` before you compile your simulation program.
     This only works for the current terminal session, if you open a new terminal, you need to run the above
     commands again. Or you can add the above three `export` commands into your shell configuration file, see
     below.

   - (permanent configuration) add the above three `export` commands into your shell configuration file:
     e.g. `~/.bashrc` for `bash` or `~/.zshrc` for `zsh`. You can run `echo $SHELL` to check your shell type,
     which will print something like `/bin/bash` on your screen. Then run `source ~/.bashrc`/`source ~/.zshrc`
     to make the changes take effect. Then you can use `galotfa` without configure every time.

6. After this step, you can compile your simulation code with `galotfa` by adding `-lgalotfa` to the compiler
   flags, e.g. `g++ ... -lgalotfa ...` or `mpicxx ... -lgalotfa ...`. In general, this flags are specified
   in Makefile or other configuration files within the simulation codes.

7. If you want to uninstall `galotfa`, run `make uninstall` in the `galotfa` repo directory.

   If the install path end up with `galotfa`, then such directory will be removed completely. Otherwise, only
   the `galotfa` library files under such directory will be removed.

---

## Usage <a href="#contents"><font size=4>(contents)</font></a> <a id="usage"></a>

### Get start with the built-in `Gadget4` fork

To get start with the built-in `Gadget4` fork, you can go through the following steps:

1. Install `galotfa` as usually, see in the previous section.
2. Go into the `Gadget4` directory in `galotfa` repo, use as a normal `Gadget4` code. Note that the built-in
   `Gadget4` fork has additional configuration and runtime parameters, which are described in the
   <a href="#fork_gadget4">Fork of `Gadget4`</a> section.
3. Copy the `galotfa.ini` from the `examples` directory into the working directory of your simulation,
   namely the directory where you run the simulation. For example, if you run `Gadget4` by command
   like `mpirun -np 32 ./Gadeget4 param.txt`, then the working directory is just the current directory.
4. Modify the parameters in the `galotfa.ini` file according to your requirement of the analysis.
5. Run the simulation, and the output files will be generated in the `output_dir` specified in the
   `galotfa.ini` file.

### Units of quantities

Due to the mutability of the simulation's internal units, so `galotfa` just use the internal units of the
simulation, keep this in mind when you use the analysis results.

### INI parameter file

Note: for INI file, the section name is <font color="red">case sensitive</font>, but the key/value name is case
insensitive.

- Available booleans: `true` and `false`, `on` and `off`, `enable` and `disable`, `yes` and `no`. This is case insensitive.
- Available value type: boolean, string(s), number(s). For numbers, `galotfa` doesn't distinguish integer and
  float, and just convert them to required types during analysis.
- Comment prefix: `#` and `;`. Any text after these characters will be ignored.
- Supported value separator: white space, `,`, `+` and `:`. Note that the name of key should not contain
  these characters otherwise the parser will raise an error.
- Unexpected additional value for a key will be illegal, e.g. `<a key for boolean> = true` is OK, but
  `<a key for boolean> = true yes` is wrong as it will make the parser to treat it as a string "true yes".

#### Runtime parameters for `galotfa`

All parameters are listed in the following table, you can click their links to see their explanation.

| Section    | Key Name                                                     | Value Type | Default       | Available Values                                          |
| ---------- | ------------------------------------------------------------ | ---------- | ------------- | --------------------------------------------------------- |
| `Global`   |                                                              |            |               |                                                           |
|            | <a href="#switch_on">`switch_on`</a>                         | Boolean    | `on`          |                                                           |
|            | <a href="#output_dir">`output_dir`</a>                       | String     | `./otfoutput` | Any valid path.                                           |
|            | <a href="#convergence_type">`convergence_type`</a>           | String     | `absolute`    | `absolute` or `relative`.                                 |
|            | <a href="#convergence_threshold">`convergence_threshold`</a> | Float      | 0.001         | $>0$, and further $<1$ if `convergence_type` = `relative` |
|            | <a href="#max_iter">`max_iter`</a>                           | Integer    | 25            | $>0$                                                      |
|            | <a href="#equal_threshold">`equal_threshold`</a>             | Float      | 1e-10         | $>0$, but should be not too large or small.               |
|            | <a href="#sim_type">`sim_type`</a>                           | String     | `galaxy`      | Only `galaxy` at present.                                 |
|            | <a href="#pot_tracer">`pot_tracer`</a>                       | Integer    |               |                                                           |
| `Pre`      |                                                              |            |               |                                                           |
|            | <a href="#recenter">`recenter`</a>                           | Boolean    | `on`          | `on` or `off`                                             |
|            | <a href="#recenter_anchors">`recenter_anchors`</a>           | Integer(s) |               | Any avaiable particle types of the simulation IC          |
|            | <a href="#region_shape">`region_shape`</a>                   | String     | `cylinder`    | `sphere`, `cylinder` or `box`.                            |
|            | <a href="#axis_ratio">`axis_ratio`</a>                       | Float      | 1.0           | $>0$                                                      |
|            | <a href="#size">`region_size`</a>                            | Float      | 20.0          | $>0$                                                      |
|            | <a href="#recenter_method">`recenter_method`</a>             | String     | `density`     | `com`, `density` or `potential`                           |
| `Model`    |                                                              |            |               |                                                           |
|            | <a href="#switch_on_m">`switch_on`</a>                       | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#filename_m">`filename`</a>                         | String     | `model`       | Any valid filename.                                       |
|            | <a href="#period_m">`period`</a>                             | Integer    | 10            | $>0$                                                      |
|            | <a href="#particle_types">`particle_types`</a>               | Integer(s) |               | Any avaiable particle types of the simulation IC          |
|            | <a href="#multiple">`multiple`</a>                           | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#classification">`classification`</a>               | Strings    | empty         | see in the <a href="#classification">text</a>             |
|            | <a href="#region_shape_m">`region_shape`</a>                 | String     | `cylinder`    | `sphere`, `cylinder` or `box`.                            |
|            | <a href="#axis_ratio_m">`axis_ratio`</a>                     | Float      | 1.0           | $>0$                                                      |
|            | <a href="#size_m">`region_size`</a>                          | Float      | 20.0          | $>0$                                                      |
|            | <a href="#align_bar">`align_bar`</a>                         | Boolean    | `on`          | `on` or `off`                                             |
|            | <a href="#image">`image`</a>                                 | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#image_bins">`image_bins`</a>                       | Integer    | 100           | $>0$                                                      |
|            | <a href="#colors">`colors`</a>                               | String(s)  |               | see in the <a href="#colors">text</a>                     |
|            | <a href="#sbar">`sbar`</a>                                   | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#bar_threshold">`bar_threshold`</a>                 | Float      | 0.15          | $(0, 1)$                                                  |
|            | <a href="#bar_major_axis">`bar_major_axis`</a>               | Boolean    | `off`         | `on` of `off`                                             |
|            | <a href="#bar_radius">`bar_radius`</a>                       | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#rmin">`rmin`</a>                                   | Float      | 0.0           | $>0$                                                      |
|            | <a href="#rmax">`rmax`</a>                                   | Float      | Region size   | $>0$                                                      |
|            | <a href="#rbins">`rbins`</a>                                 | Integer    | 20            | $>0$                                                      |
|            | <a href="#percentage">`percentage`</a>                       | Float      | 70            | $(0, 100)$                                                |
|            | <a href="#sbuckle">`sbuckle`</a>                             | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#An">`An`</a>                                       | Integer(s) |               | > 0                                                       |
|            | <a href="#inertia_tensor">`inertia_tensor`</a>               | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#dispersion_tensor">`dispersion_tensor`</a>         | Boolean    | `off`         | `on` or `off`                                             |
| `Particle` |                                                              |            |               |                                                           |
|            | <a href="#switch_on_p">`switch_on`</a>                       | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#filename_p">`filename`</a>                         | String     | `particle`    | Any valid filename.                                       |
|            | <a href="#period_p">`period`</a>                             | Integer    | 10000         | $>0$                                                      |
|            | <a href="#particle_types">`particle_types`</a>               | Integer(s) |               | Any avaiable particle types of the simulation IC          |
|            | <a href="#circularity">`circularity`</a>                     | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#circularity_3d">`circularity_3d`</a>               | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#rg">`rg`</a>                                       | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#freq">`freq`</a>                                   | Boolean    | `off`         | `on` or `off`                                             |
| `Orbit`    |                                                              |            |               | `on` or `off`                                             |
|            | <a href="#switch_on_o">`switch_on`</a>                       | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#filename_o">`filename`</a>                         | String     | `orbit`       | Any valid filename prefix.                                |
|            | <a href="#period_o">`period`</a>                             | Integer    | 1             | $>0$                                                      |
|            | <a href="#idfile">`idfile`</a>                               | String     |               | Any valid filename.                                       |
| `Group`    |                                                              |            |               |                                                           |
|            | <a href="#switch_on_g">`switch_on`</a>                       | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#filename_g">`filename`</a>                         | String     | `group`       | Any valid filename prefix.                                |
|            | <a href="#period_g">`period`</a>                             | Integer    | 10            | $>0$                                                      |
|            | <a href="#group_types">`group_types`</a>                     | String(s)  |               | (future feature)                                          |
|            | <a href="#ellipticity">`ellipticity`</a>                     | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#rmg">`rmg`</a>                                     | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#vmg">`vmg`</a>                                     | Boolean    | `off`         | `on` or `off`                                             |
| `Post`     |                                                              |            |               |                                                           |
|            | <a href="#switch_on_post">`switch_on`</a>                    | Boolean    | `off`         | `on` or `off`                                             |
|            | <a href="#filename_post">`filename`</a>                      | String     | `post`        | Any valid filename.                                       |
|            | <a href="#pattern_speed">`pattern_speed`</a>                 | Boolean    | off           | `on` or `off`                                             |

#### Explanation of parameters in each section

##### Global

- <a id="switch_on"></a>`switch_on`: whether to switch on on-the-fly analysis. If `on`, `galotfa` will analyze the
  data at specified time steps and output analysis results to the `output_dir`.
- <a id="output_dir"></a>`output_dir`: the path to output the analysis results.
- <a id="convergence_type"></a>`convergence_type`: the type of numerical convergence criterion, can be
  `absolute` or `relative`.
- <a id="convergence_threshold"></a>`convergence_threshold`: the threshold for numerical convergence check.
  - If `convergence_type` = `absolute`: the convergence criterion is $\Delta Q_i<\epsilon$ for some quantity $Q$,
    where $\epsilon$ is the `convergence_threshold`.
  - If `convergence_type` = `relative`: the convergence criterion is $\Delta Q_{i+1} / Q_{i} < \epsilon$ for some quantity $Q$,
    where $\epsilon$ is the `convergence_threshold`.
- <a id="max_iter"></a>`max_iter`: the maximum number of iterations.
- <a id="equal_threshold"></a>`equal_threshold`: the threshold for equality of two floating point numbers. For
  example, if `equal_threshold`=0.001, then two float numbers $a,b$ s.t. $|a-b|<0.001$ are considered equal. Recommended
  value is from $10^{-6}$ to $10^{-20}$.
- <a id="sim_type"></a>`sim_type`: the type of simulation, can be one of `galaxy`, `cluster`, `cosmology` and `cosmology_zoom_in`.
  At present, only `galaxy` is supported.
- <a id="pot_tracer"></a>`pot_tracer`: the particle type of the zero-mass potential tracers, which will be used to
  calculate the potential related quantities. (future feature)

##### Pre

- <a id="recenter"></a>`recenter`: whether to align the origin of coordinates to the center of the target particle(s)
  or not during calculation. Note that this alignment is only for data analysis and doesn't change the simulation
  data. This alignment will significantly affect the quality of analysis results which are sensitive to the origin
  of coordinates, such as the bar major axis. Therefore, it should always be turn on, unless you know what you are doing.
  The program will iterate until the center is convergent or reach the iteration limit (see `convergence_type`,
  `convergence_threshold` and `max_iter` in the <a href="#global">Global</a> section).
- <a id="recenter_anchors"></a>`recenter_anchors`: the particle type id(s) which are used to calculate the
  center of the system, if `recenter` = `on` then this parameter must be given at least one type, otherwise
  the program will raise an error. For example, the particle type of disk particles is 2, then you can set
  `recenter_anchors` = `2` to use the disk particles to calculate the center of the system.
  !!!! Remove this!
- <a id="region_shape"></a>`region_shape`: the program only includes the particles within a central region to do the
  system center calculation, and the shape of the region is specified by this parameter. Only effective when
  `recenter` = `on`. This parameter works together with `axis_ratio` and `region_size`. Available values:
  - `spheroid`: the region is a spheroid, or a sphere if `axis_ratio` $=$ 1, the axis of symmetry of the spheroid is
    parallel to the $z$-axis.
  - `cylinder`: the region is a cylinder with symmetry axis parallel to the $z$-axis.
  - `box`: the region is a box with sides parallel to the $x$, $y$ and $z$ axes.
- <a id="axis_ratio"></a>`axis_ratio`: specifies the axis ratio of the region, which will affect how the
  `region_size` takes effect. Only effective when `recenter` = `on`,
- <a id="size"></a>`region_size`: specifies the size of the region used to align the system center.
  Only effective when `recenter` = `on`, and has different meaning based on the value of `region_shape`:
  - `region_shape` = `sphere`: radius in the mid-plane = `region_size`, half length of the third axis = `axis_ratio`
    $\times$ `region_size`.
  - `region_shape` = `cylinder`: radius = `region_size`, half height = `axis_ratio` $\times$ `region_size`.
  - `region_shape` = `box`: $x$ and $y$ side lengths = `region_size`, $z$ side length = `axis_ratio` $\times$ `region_size`.
- <a id="recenter_method"></a>`recenter_method`: the method used to calculate the center of target particles.
  - `recenter_method` = `com`: the center is defined as the center of mass of target particles.
  - `recenter_method` = `density`: the center is defined as the center of the pixel with highest surface density,
    in the faced-on and edge-on matrix of the target particles. The size and number of pixels are determined by
    `region_size` and the `image_bins` parameter in the `Model` section. Note that if the `region_method` = `density`,
    then the `image` option in the `Model` section will be automatically turned on and `surface_density` will be
    automatically added into the `colors` parameter in the `Model` section.
  - `recenter_method` = `potential`: defined the center as the position of the most bound particle in the target
    particles, namely the particle with the lowest potential (energy). Note that this is a future feature, and
    not available at present.

##### Model

- <a id="switch_on_m"></a>`switch_on`: whether to switch on the model level analysis.
- <a id="filename_m"></a>`filename`: the filename of the model level analysis results.
- <a id="period_m"></a>`period`: the period of model level analysis, in unit of synchronized time steps in
  the simulation.
- <a id="particle_types"></a>`particle_types`: the type(s) of target particles to do the model level
  analysis. Must be given at least one type if the model level analysis is switched on.

The following 5 parameters will be deprecated in the future.

- <a id="multiple"></a>`multiple`: whether to enable multiple analysis subsets, namely analyze different
  target particles separately. If `on`, the target particles will be divided into different subsets, which are
  specified by the next parameter `classification`. Default is `multiple` = `off`, which means all target particles
  will be analyzed as a whole.
- <a id="classification"></a>`classification`: specify how to classify target particles as different
  analysis subsets. Must be given if `multiple` = `on`. The value should be strings of target types in
  each subset, for example "1&2" "3" "4&5&6" means there are 3 subsets, the first subset contains target
  particles of type 1 and 2, the second subset contains target particles of type 3, and the third subset
  contains target particles of type 4, 5 and 6.
  - Each string should be enclosed by quotation marks.
  - There could be ordinary delimiters between each string, such as white space, comma, plus sign and colon.
    For example, `"1&2" "3" "4&5&6"` and `"1&2","3","4&5&6"` are both OK.
  - However, there should has no space between, before or after the integers in each pair of quotation marks,
    otherwise the program will raise an error: `"1 & 2" "3" "4 & 5 & 6"` is illegal.
  - Different sets can share the same particle types, e.g. `"1&2" "2&3"` is legal, which means the first subset
    contains the target particles of type 1 and 2, and the second subset contains target particles of type 2 and 3.
  - The type ids should in the range of `particle_types`, otherwise the program will raise an error.
- <a id="region_shape_m"></a>`region_shape`: similar to the `region_shape` in the `Pre` section, but this one is
  used to calculate the model quantifications of target particles, can get multiple values.
  - `region_shape` = `sphere`: the region is a sphere or spheroid if `axis_ratio` $\neq$ 1, the axis of the spheroid
    is the parallel to the $z$-axis.
  - `region_shape` = `cylinder`: the region is a cylinder with symmetry axis parallel to the $z$-axis.
  - `region_shape` = `box`: the region is a box with sides parallel to the $x$, $y$ and $z$ axis.
- <a id="axis_ratio_m"></a>`axis_ratio`: similar to the `axis_ratio` in the `Pre` section, but this one is used to
  calculate the model quantifications of target particles.
- <a id="size_m"></a>`region_size`: similar to the `region_size` in the `Pre` section, but this one is used to
  - `region_shape` = `sphere`: the region is a sphere with $R=$ `region_size` if `axis_ratio` = 1. If `axis_ratio`
    $\neq$ 1, the sphere will be stretched along the $z$-axis with $R_z=$ `axis_ratio` $\times$ `region_size`.
  - `region_shape` = `cylinder`: the region is a cylinder with $R=$ `region_size`, and half height $H=$
    `axis_ratio` $\times$ `region_size`.
  - `region_shape` = `box`: the region is a cube with side length $L=$ `region_size`, and stretched along the
    $z$-axis with $L_z=$ `axis_ratio` $\times$ `region_size`.
- <a id="align_bar"></a>`align_bar`: whether to rotate the coordinates to align the $x$-axis with the bar major axis.
  This is only done when the bar is detected, namely bar strength $>$ `bar_threshold`. Note that:
  - If this option is activated then the `bar_major_axis` and `sbar` will be automatically activated.
  - The `recenter` parameter in `Pre` section should be switched on otherwise the program will raise an error.
    Of course the program can automatically turn on the `recenter` parameter if it is off, but we don't do this
    to make the user aware of what they are doing.

<font color="red">**Note:**</font> all the following bar related quantities assume that the bar is in the
$x-y$ plane. If bar is inclined w.r.t the x-y plane, these quantities may be inaccurate. The possible
correction is available with the inertia tensor, but not implemented at present.

- <a id="image"></a>`image`: whether to output the image matrices of the target particles. If activated,
  the particles will be binned into 2D matrices in the $x-y$ plane, $x-z$ plane and $y-z$ plane.
  - The bin number is specified by the `image_bins` parameter (see below).
  - The statistical quantities in each pixel are specified by the `colors` parameter (see below).
- <a id="image_bins"></a>`image_bins`: how many bins of the image matrices in each dimension. For the axis that
  may be stretched, the number of bins in such axis is `axis_ratio` $\times$ `image_bins`.
  Note that this number should not be too large, otherwise the chunk size of image matrix may overflow the
  limit chunk size of HDF5 library, which is 4GB. In general, a number in $[50, 200]$ is recommended.
- <a id="colors"></a>`colors`: If the `image` is enabled, at least one value must be given.
  - `number_density`: the number of particles in each bin.
  - `surface_density`: the surface density of the particles in each bin. The unit is $[M]/[L]^2$, $[M]$ and
    $[L]$ are the internal units of mass and length in the simulation.
  - `mean_velocity`: the mean velocity of the particles in each bin, depending on the region shape and one
    component for <font color="red">**each axis**</font> w.r.t the region shape.
  - `velocity_dispersion`: similar to `mean_velocity`, but for velocity dispersion.
- <a id="sbar"></a>`sbar`: whether to calculate the bar strength parameter $S_{\rm{bar}}$, which is defined
  as $A_2/A_0$.
- <a id="bar_threshold"></a>`bar_threshold`: the threshold to determine whether there is a bar, namely if
  $S_{\rm bar}>$ `bar_threshold`, the program will consider there is a bar.
  - In general, a range in $[0.1, 0.2]$ is recommended, but it depends on the actual situation.
- <a id="bar_major_axis"></a>`bar_major_axis`: whether to calculate the bar major axis in target particles,
  which is defined as $\frac{1}{2}\arg{(A2)}$, the argument angle of the $m$=2 Fourier component of the projected surface
  density in the $x-y$ plane. This is only done when the bar is detected, namely bar strength $>$
  `bar_threshold`.
- <a id="bar_radius"></a>`bar_radius`: whether to calculate the radius or half length of the bar, this is
  calculated with the first three methods in [Ghosh & Di Matteo 2023](https://ui.adsabs.harvard.edu/abs/2023arXiv230810948G/abstract).
  They are called $R_{\rm bar,1}$, $R_{\rm bar,2}$ and $R_{\rm bar,3}$, respectively. See more details
  in the development-manual/computation.md, all the three different methods will be calculated if this
  option is turned on.
- <a id="rmin"></a>`rmin`: the minimum radius of data points during calculating the bar radius, only
  $R_{\rm bar,2}$ is sensitive to this parameter, due to the inner most region is generally spherical,
  so the argument angle of $m=2$ Fourier component is noisy in such region. Default is 0, effective
  only when `bar_radius` = `on`.
- <a id="rmax"></a>`rmax`: the maximum radius of data points during calculating the bar radius. If not given,
  the maximum radius will be set as `region_size`, which means all particles in the analysis region will be
  included. This parameter is only effective when `bar_radius` = `on`.
- <a id="rbins"></a>`rbins`: the number of bins during calculating the bar radius. Default is 20, effective
  only when `bar_radius` = `on`.
- <a id="deg"></a>`deg`: the degree threshold to determine the location of the bar ends, only effective
  when `bar_radius` = `on`. This is the free parameter of $R_{\rm bar,1}$ in [Ghosh & Di Matteo 2023](https://ui.adsabs.harvard.edu/abs/2023arXiv230810948G/abstract).
  In general, $3^\circ\sim5^\circ$ is recommended, but it depends on the actual situation. The unit is degree,
  only effective when `bar_radius` = `on`.
- <a id="percentage"></a>`percentage`: the percentage of drop to be considered as the bar ends, the free parameter of $R_{\rm bar,3}$ in
  [Ghosh & Di Matteo 2023](https://ui.adsabs.harvard.edu/abs/2023arXiv230810948G/abstract).
  In general, $70\%\sim80\%$ is recommended, but it depends on the actual situation. Effective only when
  `bar_radius` = `on`.
- <a id="sbuckle"></a>`sbuckle`: whether to calculate the buckling strength parameter $S_{\rm{buckle}}$,  
  is defined as $\sum m_i z_i \exp(-2i \phi_i) / \sum m_i$.
- <a id="An"></a>`An`: whether to calculate the $A_n$ parameters, which is the $n$-th Fourier component of the
  projected surface density in the $x-y$ plane. Note that actually only 0 - 6 are supported, as higher order
  components are not so important and may be noisy. So if you really want to calculate $A_n$ with $n>6$,
  you need to change the code by yourself.
- <a id="inerita_tensor"></a>`inertia_tensor`: whether to calculate the inertia tensor of target particles.
- <a id="dispersion_tensor"></a>`dispersion_tensor`: whether to calculate the velocity dispersion tensor of
  target particles in each bin.
  - The principle axes of the tensor are dependent on the region shape: for example, if the region shape is
    `sphere`, the three principle axes are $\hat{r}$, $\hat{\theta}$ and $\hat{\phi}$.
  - Its spatial resolution is the same as the `image_bins`: so if `image_bins` = 100, there will be
    $100\times100\times100$ bins.

##### Particle

- <a id="switch_on_p"></a>`switch_on`: whether to enable the particle level analysis or not.
- <a id="filename_p"></a>`filename`: the filename of the particle level analysis results.
- <a id="period_p"></a>`period`: the period of particle level analysis, in unit of synchronized time steps in
  the simulation.
  <font color=red>**Note:**</font> the particle level analysis will output all information of target particles
  into a single file at each analysis time step. The size of such output file is comparable or even larger than
  a snapshot file, so the period should not be not be set too small, otherwise the output files will consume
  too much disk space. Take as an example, `period` = 5000 is a good choice for a 1e5 time steps simulation.
- <a id="particle_types"></a>`particle_types`: the type(s) of particle to do the particle level analysis.
  Must be given at least one type if the particle level analysis is enabled.
- <a id="circularity"></a>`circularity`: whether to calculate the circularity of target particles.
- <a id="circularity_3d"></a>`circularity_3d`: whether calculate the 3D circularity of target particles.
- <a id="rg"></a>`rg`: whether to calculate the guiding radii of target particles. (future feature)
- <a id="freq"></a>`freq`: whether to calculate the orbital frequency of target particles. (future feature)

##### Orbit

- <a id="switch_on_o"></a>`switch_on`: whether to enable the orbit curve log.
- <a id="filename_o"></a>`filename`: the filename of orbit curves.
- <a id="period_o"></a>`period`: the period of orbit curve log, in unit of synchronized time steps in the
  simulation. If there is only a few particles to be logged, the period can be set to a small value.
- <a id="idfile"></a>`idfile`: the particles ids' file to specify which particles to be logged. Must
  be given if the orbit curve log is enabled, otherwise the program will raise an error.
  - The particle id in this file can be separated by any one of the following delimiters: white space, new line,
    `,`, `-`, `+`, `:` and `&`.
  - Particle IDs that is not exist in the simulation will be ignored.
  - In the future, if this parameter is not given, the program will randomly select some particles to log.

#### Group (future feature)

- <a id="switch_on_g"></a>`switch_on`: whether to enable the group level analysis.
- <a id="filename_g"></a>`filename`: the filename the group level analysis results.
- <a id="period_g"></a>`period`: the period of group level analysis, in unit of synchronized time steps in
  the simulation.
- <a id="group_types"></a>`group_types`: the type(s) of group classification to do the group level analysis.
  Must be given at least one type if the group level analysis is enabled.
  - `age`: divide the particles into different age groups.
  - `metallicity`: divide the particles into different metallicity groups.
- <a id="ellipticity"></a>`ellipticity`: whether to calculate the ellipticity of target particles.
- <a id="rmg"></a>`rmg`: whether to calculate the radial metallicity gradient of target particles.
- <a id="vmg"></a>`vmg`: whether to calculate the vertical metallicity gradient of target particles.

#### Post

- <a id="switch_on_post"></a>`switch_on`: whether to enable the post analysis or not.
- <a id="filename_post"></a>`filename`: the filename of the post analysis results.
- <a id="pattern_speed"></a>`pattern_speed`: whether to calculate the pattern speed of the bar. If this
  option is enabled, the `bar_major_axis` option in the `Model` section will be automatically enabled.
- <a id="SFH"></a>`SFH`: whether to calculate the star formation history of target particles.

#### Output files (To be enriched)

Due to there may be a case of a restart simulation such as in `Gadget4`, `galotfa` will not overwrite any
existing file, but create a new file with a `-n` suffix, where `n` is an integer and starts from 1.

#### Use `galotfa` in other simulation codes

`galotfa` is based on `MPI`, and all `galotfa` APIs are designed to be used in `MPI` mode. So you need to
call `MPI_Init` before using any `galotfa` APIs.

---

## Future features

- [x] (other) add potential tracer support into the built-in simulation codes.
- [x] (other) add built-in fork of `Gadget4` with `galotfa` built-in.
- [ ] (global) output the used parameters to a separate file: galotfa-used.ini
- [ ] (global) specify different target particle types (and possible multiple analysis sets) in different analysis level.
- [ ] (pre-process) support triaxial region shape.
- [x] (model) the bar length calculation.
- [x] (model) model level analysis for different components.
- [ ] (model) DM halo spin parameter
- [ ] (particle) the guiding radius calculation.
- [ ] (particle) the orbital frequency calculation.
- [ ] (particle) the actions calculation.
- [ ] group based analysis.
- [ ] orbit curves: raw, recentered, aligned, corotating, etc.
- [ ] orbit curves: randomly selected particles.
- [ ] (post) pattern speed
- [ ] (post) star formation history
