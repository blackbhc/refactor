This documentation explains the details of computation algorithms in `galotfa`.

---

## Content <a name="content"></a>

1. <a href="#pre-process">Pre-process</a>
2. <a href="#model">Model Analysis</a>
3. <a href="#particle">Particle Analysis</a>
4. <a href="#group">Group Analysis</a>
5. <a href="#orbit">Orbit Log</a>

---

## Pre-process <a name="pre-process"></a><a href="#content"><font size=4>(content)</font></a>

### System center

The value of the system's center will be calculated with iteration, until its value converges within the
tolerance or reached the maximum iteration times, the convergence threshold and the maximum iteration times
are specified in the `Global` section of the ini file. There are three methods to calculate the system's
center:

1. As the center of mass (`recenter_method=com`): $\vec{r}_{\rm com} = \frac{\sum_i m_i \vec{r}_i}{\sum_i m_i}$,
   where $\vec{r}_i$ and $m_i$ are the position and mass of the $i$-th particle.

2. As the most bound particle's position (`recenter_method=potential`): The most bound particle is defined as
   the particle with the minimum potential energy. Note that this method is not reliable for the system with
   more than two galaxies.

3. As the densest pixel (`recenter_method=density`): the pixel of the face-on and edge-on density map with the
   maximum particle count. The size of the pixel is determined by the `image_bins` and `region_size` parameter
   in the `Model` section of the ini file.

---

## Model Analysis <a name="model"></a><a href="#content"><font size=4>(content)</font></a>

### Fourier Symmetry Modes of Surface Density

$A_n\equiv\sum_i^N {m_i\cdot\exp{(in\phi_i)}}$, $N$ is the number of particles, $m_i$ is the mass of the
$i$-th particle, $\phi_i$ is the azimuthal angle of the $i$-th particle in cylindrical coordinate, $n$ is
the order of the mode.

### Bar major axis

$\phi_{\rm bar} \equiv \frac{1}{2}\arg{A_2}$

### Bar strength

$S_{\rm bar}=|A_2/A_0|$

### Buckling strength

$S_{\rm buckle}=|\dfrac{\sum_i^N {z_i m_i\cdot\exp{(2i\phi_i)}}}{A_0}|$

### Dispersion tensor

$\sigma_{ij}^2=<v_i\,v_j> - <v_i><v_j>$, in which $v_i$ is the $i$-th component of the velocity at every pixel
of the specified analysis region: summation is over the local pixel.

### Inertia tensor

$I_{xx}=\sum_i^N {m_i(y_i^2+z_i^2)}$

$I_{yy}=\sum_i^N {m_i(x_i^2+z_i^2)}$

$I_{zz}=\sum_i^N {m_i(x_i^2+y_i^2)}$

$I_{xy}=I_{yx}=-\sum_i^N {m_i\,x_i\,y_i}$

$I_{xz}=I_{zx}=-\sum_i^N {m_i\,x_i\,z_i}$

$I_{yz}=I_{zy}=-\sum_i^N {m_i\,y_i\,z_i}$

The summation is over the whole region.

### Bar radius (in the image plane)

The bar radius or half-length is the radius of the circle enclosing the bar region. Such region is
actually not well defined. Algorithms used here is referred to [Ghosh & Di Matteo 2023](https://ui.adsabs.harvard.edu/abs/2023arXiv230810948G/abstract).

- $R_{\rm bar,1}$: the radial extent within which the phase-angle of the $m=2$ Fourier moment ($\phi_2$)
  remains constant within some threshold, such as $3^\circ \sim 5^\circ$. The exact value of such threshold is
  a free parameter, which is specified in the ini file. Note that $\Delta\phi_2$ is w.r.t the bar major axis.
- $R_{\rm bar,2}$: the radial extent where the bar strength $S_{\rm bar}$ reaches its maximum.
- $R_{\rm bar,3}$: the radial extent where the bar strength $S_{\rm bar}$ drops to $70\%$ of its maximum value,
  which is outside the maximum point. The exact value of the percentage is a free parameter specified in the
  ini file.

---

## Particle Analysis <a name="particle"></a><a href="#content"><font size=4>(content)</font></a>

### Circularity

$\epsilon=\frac{L_z}{L_c(E)}$, where $L_z$ is the $z$-component of the angular momentum, and $L_c$ is the angular
momentum of a circular orbit at the particle's radius with the same energy $E$. This quantity can quantify the
extent of deviation that the particle's orbit deviates from a in-plane circular orbit.

$\epsilon_{\rm 3D}=\frac{L}{L_c(E)}$, similar to the above, but $L$ is the magnitude of the particle's angular
momentum. This quantity can quantify the deviation of the particle's orbit deviates from a circular orbit
(possibly out-of-plane).
