#
# Sample code for pygmvis
# To use pygmvis, place the generated files (on Windows: pygmvis.lib, pygmvis.\*.pyd, Qt\*.dll) in the same folder as your python code.
#

import pygmvis
import matplotlib
import matplotlib.pyplot as plt
matplotlib.use('TkAgg') # Backend Qt5Agg creates problems when using GMVis, so rather use TkAgg or something else

# Create the visualizer. Parameters: 1) Use asynchronous rendering (result returned by callback), 2) width, 3) height
vis = pygmvis.create_visualizer(False, 500, 500)

# The vis-object provides access to all functions shown in gmvis/pylib/Visualizer.cpp
# Set camera position automatically to given mixture model
vis.set_camera_auto(True)
# Enable density rendering
vis.set_density_rendering(True)
# Pass Mixture from path. Second parameter states that the weights are given as amplitudes rather than prior weights that sum to one
vis.set_gaussian_mixtures_from_paths(["data/bed_0001.gma.ply"], False)
# Render the result. Parameter is a number used to identify render calls in asynchronous mode. The result is an array of shape (b, r, h, w, 4),
# where b is the number of mixtures, r the number of enabled visualizations, h the height, w the width, and 4 is the number of color channels.
res = vis.render(0)[0, 0]
# When a visualizer is not required anymore, finish should be called to properly shutdown!
vis.finish()

# Plot result
f1 = plt.figure("Rendering")
f1.figimage(res)
plt.show()