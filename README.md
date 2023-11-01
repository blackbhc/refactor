`galotfa`: <font size=4>**gal**</font>actic <font size=4>**o**</font>n-<font size=4>**t**</font>he-<font size=4>**f**</font>ly <font size=4>**a**</font>nalysis, is a library for on-the-fly analysis of disk galaxy simulations.

---

## <a id="contents">Contents</a>

- <a href="#feature">Features of `galotfa`</a>
- <a href="#install">Installation</a>
- <a href="#scheme">Design scheme</a>
- <a href="#usage">Usage</a>

---

## Features of `galotfa` <a href="#contents"><font size=4>(contents)</font></a> <a id="feature"></a>

1. Out-of-the-box: in general no need to modify the simulation code, just run some demos with simplest
   steps. If you feel hard to use some part of `galotfa`, please let us know.
2. User friendly program API and usage guidance: although we hope most functions of `galotfa` can be
   used out-of-the-box, we also provide detailed guidance for users to use `galotfa` in their own customized
   way.
3. Extensible: easy to add new analysis functions or apply to new simulation code which follows the general
   simulation convention. Besides, although this project is concentrated on disk galaxy simulations, it can
   also be easily extended to other types of simulations, such as cluster simulations, cosmology simulations,
   etc.
4. Notice us if you need more wonderful features: you-complete-me flavor, you can make this project better.
   We are also happy to merge your code into this project if you want.
5. Low dependency on other libraries: only standard libraries or included in the project.
6. Fast: use MPI and design to, a level of **~5%?** more CPU time during a run of simulation.
7. by the hand: `galotfa` repo also provides some extended version of widely used simulation codes,
   with `galotfa` built-in. You can also add `galotfa` in any simulation code by yourself or submit it.
8. Open: we welcome new participants who are interested in improve this project.

---

## Installation <a href="#contents"><font size=4>(contents)</font></a> <a id="install"></a>

### Dependencies

First, you need to check the following dependencies

- `make`: `galotfa` is organized by `make`, run `which make` to check the installation of `make` in your system.
  If you don't have `make`, please install it first.

- a `c++` compiler with `c++11` support, e.g. `g++`>4.8.5 or `clang++`>3.9.1 are recommended.

- dependent libraries:

  - any `MPI` library.
  - `gsl` library.

### Download and install step by step

1. clone the `galotfa` repo with `git`: run `git clone -b main https://github.com/blackbhc/galotfa --depth=1`

   If you don't have `git`, try `wget -O- https://github.com/blackbhc/galotfa/archive/main.zip | tar xz`.

2. run `cd galotfa`

3. run `make build mode=release type=header-only`.

   - the `mode` option can be `release` or `debug`, default is `release` which has `-O3` optimization mode.
     `debug` make the compiled library includes debug symbols for debugging which is only useful for developers.

   - the `type` option can be `header-only`, `static`, `shared`, `all`:

     - `header-only`: only copy the header files, no library files.
     - `static`: build the static library.
     - `shared`: build the shared library.
     - `all` = `static` + `shared`.

   Note:

   - (1) header-only is the default option, which is recommended for the first time installation.
     Such option will define a `header-only` macro in the `galotfa` header files, which will make the
     library can be used without linking to the library files.
   - (2) if you encounter any error during the installation, please check the error message and run
     `make clean` to clean the build files, then run `make build` again.
   - The header only mode is only available for `C++` compiler, and the `C` compiler will not be able to
     compile the `galotfa` header files. Therefore, to use `galotfa` in a simulation code written in `C`,
     the `type` option should be `static` or `shared`.

4. run `make -p <path/to/install>`:

   Create the directory for the installation of `galotfa`, if it already exists, skip this step.

5. run `make install prefix=</path/to/install>`:

   Install the `galotfa` to the directory specified by `prefix`, which should be the same as
   the one you specified in the last step.

6. After configure the `CPATH`, `LIBRARY_PATH` and `LD_LIBRARY_PATH` environment variables,
   you can use `galotfa` in your project.:

   - (temporary) run `export CPATH=$CPATH:<prefix>/include`, `export LIBRARY_PATH=$LIBRARY_PATH:<prefix>/lib`,
     `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<prefix>/lib` before you compile your project every time.

   - (permanent) add the above three `export` commands into your shell configuration file:

     e.g. `~/.bashrc` for `bash` or
     `~/.zshrc` for `zsh`. You can run `echo $SHELL` to check your shell type, which will print something
     like `/bin/bash` to your terminal. Then run `source ~/.bashrc`/`source ~/.zshrc` to make the changes take effect.
     Then you can use `galotfa` without configure every time.

7. If you want to uninstall `galotfa`, run `make uninstall` in the `galotfa` repo directory.

   If the install path contain `galotfa`, then such directory will be removed completely. Otherwise, only
   the `galotfa` library files under such directory will be removed.

---

## Usage <a href="#contents"><font size=4>(contents)</font></a> <a id="usage"></a>

### Get start with examples

### Complete illustrations

#### Units of quantities

Due to the mutability of the simulation's internal units, the units of analysis results are based on the
simulation internal units, keep this in mind when you use the analysis results.

#### INI parameter file

Note: the section name is case sensitive, but the key/value name is case insensitive.

- available boolean: case insensitive `true` and `false`, `on` and `off`, `enable` and `disable`, `yes` and `no`.
- available value type: boolean, string(s), number(s). For numbers, there is no difference between integer and
  float, but the value(s) will be converted to required type of the target parameter.
- comment prefix: `#` and `;`.
- supported value separator: white space, `,`, `-`, `+`, `:` and `&`. Note: the name of key can not contain these characters.
- unexpected additional value for a key will be illegal, e.g. `<a key for boolean> = true yes` will make the parser to detect
  the value of the key as `true yes`, which is in string type and may cause error in the following parsing.

#### List of parameters for `galotfa`

All the parameters are listed below, and their function is indicated by the name, or you can click the link
to see their explanation.

| Section    | Key Name                                                     | Value Type | Default       | Available Values                                            |
| ---------- | ------------------------------------------------------------ | ---------- | ------------- | ----------------------------------------------------------- |
| `Global`   |                                                              |            |               |                                                             |
|            | <a href="#switch">`switch`</a>                               | Boolean    | `on`          |                                                             |
|            | <a href="#output_dir">`output_dir`</a>                       | String     | `./otfoutput` | Any valid path.                                             |
|            | <a href="#particle_types">`particle_types`</a>               | Integer(s) |               | Based on your IC of simulation                              |
|            | <a href="#convergence_type">`convergence_type`</a>           | String     | `absolute`    | `absolute` or `relative`.                                   |
|            | <a href="#convergence_threshold">`convergence_threshold`</a> | Float      | 0.001         | $(0, 1)$ if `convergence_type` = `relative`, otherwise $>0$ |
|            | <a href="#max_iter">`max_iter`</a>                           | Integer    | 25            | $>0$                                                        |
|            | <a href="#equal_threshold">`equal_threshold`</a>             | Float      | 1e-10         | $>0$, but not too large.                                    |
|            | <a href="#sim_type">`sim_type`</a>                           | String     | `galaxy`      | Only support `galaxy` at present.                           |
|            | <a href="#pot_tracer">`pot_tracer`</a>                       | Integer    |               |                                                             |
| `Pre`      |                                                              |            |               |                                                             |
|            | <a href="#recenter">`recenter`</a>                           | Boolean    | `on`          | `on` or `off`                                               |
|            | <a href="#region_shape">`region_shape`</a>                   | String     | `cylinder`    | `sphere`, `cylinder` or `box`.                              |
|            | <a href="#ratio">`ratio`</a>                                 | Float      | 1.0           | $>0$                                                        |
|            | <a href="#size">`region_size`</a>                            | Float      | 20.0          | $>0$                                                        |
|            | <a href="#recenter_method">`recenter_method`</a>             | String     | `density`     | `com`, `density` or `potential`                             |
|            | <a href="#align_bar">`align_bar`</a>                         | Boolean    | `on`          | `on` or `off`                                               |
| `Model`    |                                                              |            |               |                                                             |
|            | <a href="#switch_m">`switch`</a>                             | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#filename_m">`filename`</a>                         | String     | `model`       | Any valid filename prefix.                                  |
|            | <a href="#period_m">`period`</a>                             | Integer    | 10            | $>0$                                                        |
|            | <a href="#region_shape_m">`region_shape`</a>                 | String     | `cylinder`    | `sphere`, `cylinder` or `box`.                              |
|            | <a href="#ratio_m">`ratio`</a>                               | Float      | 1.0           | $>0$                                                        |
|            | <a href="#size_m">`region_size`</a>                          | Float      | 20.0          | $>0$                                                        |
|            | <a href="#image">`image`</a>                                 | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#image_bins">`image_bins`</a>                       | Integer    | 100           | $>0$                                                        |
|            | <a href="#colors">`colors`</a>                               | String(s)  |               | see in the <a href="#colors">text</a>                       |
|            | <a href="#bar_major_axis">`bar_major_axis`</a>               | Boolean    | `off`         | `on` of `off`                                               |
|            | <a href="#bar_length">`bar_length`</a>                       | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#sbar">`sbar`</a>                                   | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#sbuckle">`sbuckle`</a>                             | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#An">`An`</a>                                       | Integer(s) |               | > 0                                                         |
|            | <a href="#inertia_tensor">`inertia_tensor`</a>               | Boolean    | `off`         | `on` or `off`                                               |
| `Particle` |                                                              |            |               |                                                             |
|            | <a href="#switch_p">`switch`</a>                             | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#filename_p">`filename`</a>                         | String     | `particle`    | Any valid filename prefix.                                  |
|            | <a href="#period_p">`period`</a>                             | Integer    | 10000         | $>0$                                                        |
|            | <a href="#circularity">`circularity`</a>                     | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#circularity_3d">`circularity_3d`</a>               | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#rg">`rg`</a>                                       | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#freq">`freq`</a>                                   | Boolean    | `off`         | `on` or `off`                                               |
| `Orbit`    |                                                              |            |               | `on` or `off`                                               |
|            | <a href="#switch_o">`switch`</a>                             | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#filename_o">`filename`</a>                         | String     | `orbit`       | Any valid filename prefix.                                  |
|            | <a href="#period_o">`period`</a>                             | Integer    | 1             | $>0$                                                        |
|            | <a href="#idfile">`idfile`</a>                               | String     |               | Any valid filename.                                         |
| `Group`    |                                                              |            |               |                                                             |
|            | <a href="#switch_g">`switch`</a>                             | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#filename_g">`filename`</a>                         | String     | `group`       | Any valid filename prefix.                                  |
|            | <a href="#period_g">`period`</a>                             | Integer    | 10            | $>0$                                                        |
|            | <a href="#group_types">`group_types`</a>                     | String(s)  |               | (future feature)                                            |
|            | <a href="#ellipticity">`ellipticity`</a>                     | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#rmg">`rmg`</a>                                     | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#vmg">`vmg`</a>                                     | Boolean    | `off`         | `on` or `off`                                               |
| `Post`     |                                                              |            |               |                                                             |
|            | <a href="#switch_post">`switch`</a>                          | Boolean    | `off`         | `on` or `off`                                               |
|            | <a href="#filename_post">`filename`</a>                      | String     | `post`        | Any valid filename prefix.                                  |
|            | <a href="#pattern_speed">`pattern_speed`</a>                 | Boolean    | off           | `on` or `off`                                               |

#### Explanation of parameters

##### Global

This section specify some parameters that control the behaviour of `galotfa` on the machine.

- <a id="switch"></a>`switch`: whether to enable the demo mode or not. If `on`, `galotfa` will only run for a few steps
  and output some demo files to the `output_dir`. This option is only for test purpose or may be useful for some special cases.
- <a id="output_dir"></a>`output_dir`: the path to store the output files, create it if not exist.
- <a id="particle_types"></a>`particle_types`: the type(s) of target particles types to do the on-the-fly analysis, must be
  given at least one type, otherwise the program will raise an error.
- <a id="convergence_type"></a>`convergence_type`: the type of convergence criterion for the on-the-fly analysis.
- <a id="convergence_threshold"></a>`convergence_threshold`: the threshold for numerical convergence during the
  on-the-fly analysis.
  - `convergence_type` = `absolute`: the convergence criterion is $\Delta$ $Q\<\epsilon$ for some quantity $Q$,
    where $\epsilon$ is the `convergence_threshold`.
  - `convergence_type` = `relative`: the convergence criterion is $\Delta Q / Q \< \epsilon$ for some quantity $Q$,
    where $\epsilon$ is the `convergence_threshold`.
- <a id="max_iter"></a>`max_iter`: the maximum number of iterations during analysis.
- <a id="equal_threshold"></a>`equal_threshold`: the threshold for equality of two floating point numbers, e.g.
  if the threshold=0.001, then two float numbers that $|a-b|<0.001$ are considered equal. Recommended value is
  $1e-6$ to $1e-40$, and should not less than the lowest precision of the floating point number in your system.
- <a id="sim_type"></a>`sim_type`: the type of simulation, e.g. `galaxy`, `cluster`, `cosmology` and `cosmology_zoom_in`.
  At present, only `galaxy` is supported.
- <a id="pot_tracer"></a>`pot_tracer`: the particle type id of potential tracer particles, which will be used to
  calculate the potential of the target particles. (future feature)

##### Pre

This section is about the pre-processing of the simulation data before some concrete analysis, such as calculate the
center of the target particles, calculate the bar major axis (if exist) and align the bar major axis to the $x$-axis.

- <a id="recenter"></a>`recenter`: whether to recenter the target particles to the center the target
  particle(s) or not, note the recenter is only for the on-the-fly analysis, and will not change the simulation data.
  The parameter will significantly affect the result of the on-the-fly analysis that is sensitive to the origin of
  coordinates, such as the bar major axis, the pattern speed, etc. Therefore, it's recommended to always turn on this
  option, unless you know what you are doing.
- <a id="region_shape"></a>`region_shape`: only meaningful when `recenter` = `on`, the shape of the region
  to calculate the center of the target particles, which will affect how the `region_size` is interpreted (see below).
  - `region_shape` = `sphere`: the region is a sphere or spheroid if `ratio` $\neq$ 1, the axis of the spheroid is the
    parallel to the $z$-axis.
  - `region_shape` = `cylinder`: the region is a cylinder with symmetry axis parallel to the $z$-axis.
  - `region_shape` = `box`: the region is a box with sides parallel to the $x$, $y$ and $z$ axis.
- <a id="ratio"></a>`ratio`: only meaningful when `recenter` = `on`, the ratio of the region's characteristic
  lengths, which will affect how the `region_size` is interpreted.
- <a id="size"></a>`region_size`: only meaningful when `recenter` = `on`, the size of the region to calculate
  the center of the target particles, which will
  - `region_shape` = `sphere`: the region is a sphere with $R=$ `region_size` if `ratio` = 1. If `ratio` is not 1,
    the sphere will be stretched along the $z$-axis with $R_z=$ `ratio` $\times$ `region_size`.
  - `region_shape` = `cylinder`: the region is a cylinder with $R=$ `region_size`, and half height $H=$
    `ratio` $\times$ `region_size`.
  - `region_shape` = `box`: the region is a cube with side length $L=$ `region_size`, and stretched along the
    $z$-axis with $L_z=$ `ratio` $\times$ `region_size`.
- <a id="recenter_method"></a>`recenter_method`: the method to calculate the center of the target particles,
  with iteration if necessary (see `convergence_type` and `convergence_threshold`).
  - `recenter_method` = `com`: the center is defined as the center of mass of the target particles.
  - `recenter_method` = `density`: the center is defined as the pixel of the highest surface density of the target
    particle(s), the size of the pixel is determined by `image_size` in the `Model` section.
  - `recenter_method` = `potential`: future feature.
- <a id="align_bar"></a>`align_bar`: whether rotate the coordinates to align the $x$-axis to the bar major axis,
  this option is only available when the bar is detected. It's may be useful to align the bar major axis to the
  $x$-axis for some analysis or visualization.

##### Model

The model level on-the-fly analysis of the target particles. The most common case at present is a disk galaxy.

- <a id="switch_m"></a>`switch`: whether to enable the model level analysis or not.
- <a id="filename_m"></a>`filename`: the filename of the output file of the model level analysis, the suffix `.hdf5`
  will be added automatically so you only need to specify the prefix of the filename.
- <a id="period_m"></a>`period`: the period of model level analysis, in unit of synchronized time steps in
  simulation.
- <a id="region_shape_m"></a>`region_shape`: similar to the `region_shape` in the `Pre` section, but this one is
  used to calculate the model quantifications of the target particles, can get multiple values.
  - `region_shape` = `sphere`: the region is a sphere or spheroid if `ratio` $\neq$ 1, the axis of the spheroid is the
    parallel to the $z$-axis.
  - `region_shape` = `cylinder`: the region is a cylinder with symmetry axis parallel to the $z$-axis.
  - `region_shape` = `box`: the region is a box with sides parallel to the $x$, $y$ and $z$ axis.
- <a id="ratio_m"></a>`ratio`: similar to the `ratio` in the `Pre` section, but this one is used to calculate the
  model quantifications of the target particles.
- <a id="size_m"></a>`region_size`: similar to the `region_size` in the `Pre` section, but this one is used to
  - `region_shape` = `sphere`: the region is a sphere with $R=$ `region_size` if `ratio` = 1. If `ratio` $\neq$ 1, the sphere
    will be stretched along the $z$-axis with $R_z=$ `ratio` $\times$ `region_size`.
  - `region_shape` = `cylinder`: the region is a cylinder with $R=$ `region_size`, and half height $H=$
    `ratio` $\times$ `region_size`.
  - `region_shape` = `box`: the region is a cube with side length $L=$ `region_size`, and stretched along the $z$-axis with
    $L_z=$ `ratio` $\times$ `region_size`.
- <a id="image"></a>`image`: whether to output the image matrices of the target particles.
  - The particles will be divided into bins in each axis (according to the `region_shape`) and do some statistics
    in each bin, such as the mean value of some quantity, the number of particles in each bin, etc. The bin number
    is specified by the `image_bins` parameter (see below).
  - For each region type, there will be 3 image matrices by different combination of the 3 axes, e.g. for a
    `region_shape` = `box`, there will be 3 image matrices for the $x-y$ plane, $x-z$ plane and $y-z$ plane.
  - The quantities of the image are specified by the `colors` parameter in the view of color coded (see below).
    (The name `image` may be changed in the future, as its meaning is not so clear.)
- <a id="image_bins"></a>`image_bins`: how many bins of the image matrices in each dimension, for the axis that
  may be stretched, the number of bins in such axis is also determined by the `ratio` parameter.
- <a id="colors"></a>`colors`:
  At least one color must be given, if the `image` is enabled, otherwise the program will raise an error.
  - `particle_number`: the number of particles in each bin.
  - `surface_density`: the surface density of the particles in each bin. The unit is $\[M\]/\[L\]^2$, $\[M\]$ and
    $\[L\]$ are the internal unit of mass and length in the simulation, the same below.
  - `mean_velocity`: the mean velocity of the particles in each bin, one component for each axis.
  - `dispersion`: the velocity dispersion of the particles in each bin, one component for each axis.
  - `dispersion_tensor`: the velocity dispersion tensor of the particles in each bin.
- <a id="bar_major_axis"></a>`bar_major_axis`: whether calculate the bar major axis in the target particles,
  if detected a bar, defined as the phase angle of the $m$=2 Fourier component of the surface density after
  projection into the equatorial plane, $\arg(A_2)$.
- <a id="bar_length"></a>`bar_length`: whether calculate the bar length in the target particles,
  if detected a bar.
- <a id="inerita_tensor"></a>`inertia_tensor`: whether calculate the inertia tensor of the target particles.
- <a id="sbar"></a>`sbar`: whether calculate the bar strength parameter, where $S\_\rm{bar}$ is defined
  as $A_2/A_0$.
- <a id="sbuckle"></a>`sbuckle`: whether calculate the buckling strength parameter, where $S\_{\rm{buckle}}$
  is defined as $\sum m_i z_i \exp(-2i \phi_i) / \sum m_i$.
- <a id="An"></a>`An`: whether calculate the $A_n$ parameters, where $A_n$ is the $n$-th Fourier component of the
  surface density after projection into the equatorial plane.

##### Particle

- <a id="switch_p"></a>`switch`: whether to enable the particle level analysis or not.
- <a id="filename_p"></a>`filename`: the filename of the output file of the particle level analysis, the suffix `.hdf5`
- <a id="period_p"></a>`period`: the period of particle level analysis, in unit of synchronized time steps in
  simulation.
  <font color=red>**Note:**</font> the particle level analysis will output all information of the target particles
  into a single file at each analysis time step, the size of such output file is comparable to the snapshot file,
  so the period should not be not be set too small, otherwise the output files will consume too much disk space.
  e.g. `period` = 5000 is a good choice for a 1e5 time steps simulation, 1e5 is a magnitude for a 10Gyr simulation
  in Gadget4 with default units.
  will be added automatically so you only need to specify the prefix of the filename.
- <a id="circularity"></a>`circularity`: whether calculate the circularity of the target particles.
- <a id="circularity_3d"></a>`circularity_3d`: whether calculate the 3D circularity of the target particles.
- <a id="rg"></a>`rg`: whether calculate the guiding radius of the target particles. (future feature)
- <a id="freq"></a>`freq`: whether calculate the orbital frequency of the target particles. (future feature)

##### Orbit

This part is about a trivial target: log the orbital curves of the target particles. For example, trace the orbit
of stars that contribute to the bar, or the spiral arms, etc.

- <a id="switch_o"></a>`switch`: whether to enable the orbit curve log.
- <a id="filename_o"></a>`filename`: the filename of the output file of the orbit curve log, the suffix `.hdf5`
  will be added automatically so you only need to specify the prefix of the filename.
- <a id="period_o"></a>`period`: the period of orbit curve log, in unit of synchronized time steps in simulation.
  If there is no too much particles to trace, the period can be set to a small value, e.g. 1, which means log the
  position of the target particles at every synchronized time step.
- <a id="idfile"></a>`idfile`: the path to an ASCII file of particles id of the target particle to trace, must
  be given if the orbit curve log is enabled, otherwise the program will raise an error. The particle type
  of this part can be any type of particles in the simulation, not restricted to the target particles specified
  by the `particle_types` parameter in the `Global` section.
  - The particle id in this file can be separated by any of the following characters: white space, new line,
    `,`, `-`, `+`, `:` and `&`.
  - Particle ID that is not exist in the simulation will be ignored, with some warning message.

#### Group (future feature)

This level is designed to do the on-the-fly analysis of particle groups/sets. For example, the different
stellar populations in a galaxy: binned according their ages, metallicity, etc. Or different type of galaxies
in cosmology simulations: central and satellite galaxies in a cluster, or elliptical and spiral galaxies based
on their morphology.

- <a id="switch_g"></a>`switch`: whether to enable the group level analysis or not.
- <a id="filename_g"></a>`filename`: the filename of the output file of the group level analysis, the suffix `.hdf5`
  will be added automatically so you only need to specify the prefix of the filename.
- <a id="period_g"></a>`period`: the period of group level analysis, in unit of synchronized time steps in
  simulation.
- <a id="group_types"></a>`group_types`: the type(s) of group particles to do the on-the-fly analysis, must be
  given at least one type if the group level analysis is enabled, otherwise the program will raise an error.
  - `age`: divide the particles into different groups by their age.
  - `metallicity`: divide the particles into different groups by their metallicity.
- <a id="ellipticity"></a>`ellipticity`: whether calculate the ellipticity of the target particles.
- <a id="rmg"></a>`rmg`: whether calculate the radial metallicity gradient of the target particles.
- <a id="vmg"></a>`vmg`: whether calculate the vertical metallicity gradient of the target particles.

#### Post

This part is designed to do some by-the-hand post analysis, such as calculate the bar pattern speed of the bar.

- <a id="switch_post"></a>`switch`: whether to enable the post analysis or not.
- <a id="filename_post"></a>`filename`: the filename of the output file of the post analysis, the suffix `.hdf5`
  will be added automatically so you only need to specify the prefix of the filename.
- <a id="pattern_speed"></a>`pattern_speed`: calculate the pattern speed of the bar. If this option is enabled,
  the `bar_major_axis` option in the `Model` section will be automatically enabled.
- <a id="SFH"></a>`SFH`: calculate the star formation history of the target particles. (future feature)

#### Output files

Due to there may be a case of a restart simulation, `galotfa` will not overwrite any existing file,
but create a new file with a `-n` suffix that start from 1, where `n` is the smallest integer that make the new file
name not exist. In this way, the suffix can be served as a restart index.

#### Use `galotfa` in general simulation codes

`galotfa` is based on `MPI`, and all `galotfa` APIs are designed to be used in `MPI` mode. So you need to
call `MPI_Init` before using any `galotfa` APIs.

<font color=red>**Note:**</font> `galotfa` is designed to be used in `MPI` mode, so you need to call `MPI_Init`
before using any `galotfa` APIs.

---

## Design scheme <a href="#contents"><font size=4>(contents)</font></a> <a id="scheme"></a>

At particular synchronized time steps:

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

Every box is a functional module with independent implementation, which is class or a part of a class.
The details of them are illustrated in <a href="#code">Code structure</a>. The connection lines between
the boxes stands for the APIs between such modules. Expect the above modules, `galotfa` also uses a standalone
`INI` parameter file to control the behaviours of all modules and APIs in the preceding workflow.

## Future features

- [ ] (other) add built-in fork of common simulation codes with `galotfa` built-in.
- [ ] (other) add potential tracer support into the common simulation codes.
- [ ] (model) the bar length calculation.
- [ ] (particle) the guiding radius calculation.
- [ ] (particle) the orbital frequency calculation.
- [ ] (particle) the actions calculation.
- [ ] group based analysis.
- [ ] orbit curves
- [ ] (post) pattern speed
- [ ] (post) star formation history
