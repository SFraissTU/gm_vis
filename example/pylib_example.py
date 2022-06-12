#
# This script requires the built pygmvis.lib, the built pygmvis.*.pyd file, and the Qt-dlls in the same directory

import pygmvis
import matplotlib
import matplotlib.pyplot as plt
matplotlib.use('TkAgg') # Backend Qt5Agg makes problems when using GMVis, so rather use TkAgg or something else


vis = pygmvis.create_visualizer(False, 500, 500)
vis.set_camera_auto(True)
vis.set_density_rendering(True)
vis.set_gaussian_mixtures_from_paths([r"data/c_30fix.ply"], True)
res = vis.render(0)[0, 0]
vis.finish()

f1 = plt.figure("Rendering")
f1.figimage(res)
plt.show()

