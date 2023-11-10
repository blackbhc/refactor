This documentation explains the details of computation in `galotfa`, namely the formulae and algorithms used
in the codes.

---

## Content <a name="content"></a>

1. <a href="#pre-process">Pre-process</a>
2. <a href="#model">Model Analysis</a>
3. <a href="#particle">Particle Analysis</a>
4. <a href="#group">Group Analysis</a>
5. <a href="#orbit">Orbit Log</a>

---

## Pre-process <a name="pre-process"></a><a href="#contents"><font size=4>(content)</font></a>

### System center

The calculation of the system center is only over the target particles in the recenter region with specified
particle types. (See `Usage` part in `README.md`)

Its value will be calculated by specified method, until its value converges within the tolerance or reached
the maximum iteration number, the convergence threshold and maximum iteration number are specified in the
`Global` section of the ini file.

#### As center of mass (`recenter_method=com`)

The center of mass of the system is defined as $\vec{r}_{\rm cm} = \frac{\sum_i m_i \vec{r}_i}{\sum_i m_i}$
where $\vec{r}_i$ and $m_i$ are the position and mass of the $i$-th particle, respectively.

#### As the most bound particle (`recenter_method=potential`)

The most bound particle is defined as the particle with the minimum potential energy. Note that this
method is not reliable for the system with two or more satellites.

#### As the densest pixel (`recenter_method=density`)

The center of the densest pixel is the pixel with maximum number density, where its exact value is the
center of such pixel. Note that this method is sensitive to the image matrix's pixel size, which is
inferred from `image_bins` in the `Model` section of the ini file.

## Model Analysis <a name="model"></a><a href="#contents"><font size=4>(content)</font></a>

### Density Fourier Symmetry Mode

$A_n=\sum_i^N {m_i\cdot\exp{(in\phi_i)}}$, $N$ is the number of particles, $m_i$ is the mass of the
$i$-th particle, $\phi_i$ is the azimuthal angle of the $i$-th particle in cylindrical coordinate, $n$ is
the order of the mode.

### Bar strength

$S_{\rm bar}=|A_2/A_0|$

### Buckling strength

$S_{\rm buckle}=|\sum_i^N {z_i\,m_i\cdot\exp{(2i\phi_i)}}/A_0|$
