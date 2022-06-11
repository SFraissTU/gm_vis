# Gaussian Mixture Model visualization toolkit

This toolkit was written as part of the diploma thesis [Construction and Visualization of Gaussian Mixture Models from Point Clouds for 3D Object Representation](https://www.cg.tuwien.ac.at/research/publications/2022/FRAISS-2022-CGMM/FRAISS-2022-CGMM-thesis.pdf).
Its main features are an ellipsoid visualization and a density visualization of 3D Gaussian Mixtures. These visualization are provided in both a standalone graphical user interface and a Python interface.

Current dependencies are Eigen (included in repo), Anaconda, Qt (5 or 6) and Pybind11 (see CMakeLists.txt).

## GUI functionality
A short description of the visualizer's features in the graphical user interface build (CMake target visualizer):
* Loading data:
** *File->Load Pointcloud*: Loads a pointcloud in the OFF-format (see data-folder for examples)
** *File->Load Mixture Model*: Loads a Gaussian mixture model in the PLY-format. The given prior weights will be divided by the their sum during loading.
** *File->Load Mixture with Amplitudes*: Loads a Gaussian mixture where Gaussian amplitudes are given instead of prior weights (the amplitude is the highest value of the Gaussian, e.g. priorweight / sqrt((2pi)^3 |Sigma|) )
* In the *Display*-section on the left, the available visualization techniques can be enabled/disabled. In the section on the bottom of the window, the visualization settings may be changed. The following visualizations are available:
** *Pointcloud*: Displays the loaded point cloud (if any)
** *Gaussian Positions*: Displays points on the mean positions of the Gaussians. This is useful for detecting small Gaussians which may not be visible in the other visualizations. In the settings section, their colors might be changed to correspond to their prior weights or amplitudes. Also, a gray background option is available to better see Gaussians of all colors.
** *Ellipsoids*: Displays one ellispoid per Gaussian. In the setting section, their colors might be changed to correspond to their prior weights or amplitudes. Also, a gray background option is available to better see Gaussians of all colors. Per default, each ellipsoid is the sets of all points which have a Mahalanobis distance of 1 to the Gaussian's mean. An alternative calculation method is available by clicking the button "Render iso-ellipsoids", however this is experimental and not recommended!
** *Density*: An additive density visualization. In the settings, the brightness intensity may be changed via the slider or manually through the min/max-values. These values define which pixel values are mapped to black and white. If the button *Symmetric Lock* is active each change of the max or min value will adapt the other value to its negative value. If *Logarithmic* is checked, the logarithmic values will be used instead. Per default, the settings are reset, if a new mixture is loaded. If the button *Lock values for new mixture* is checked, the values will not be changed when loading a new mixture. *Reset values* will reset them to their default value. *Min/Max Render* will set the highest and lowest pixel values as min/max-values. Different RenderModes are available, "Fast (Projection)" should work most times. If the results look wrong, the slower "Exact" mode can be used instead. The "Fast"-Methods require a set Acceleration-Threshold, which is usually chosen automatically.
