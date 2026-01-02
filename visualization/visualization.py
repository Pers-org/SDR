import numpy as np
from vispy import scene, app

canvas = scene.SceneCanvas(keys='interactive', show=True)
view = canvas.central_widget.add_view()
view.camera = 'panzoom'

x = np.linspace(0, 10, 1000)
y = np.sin(x)

scene.Line(pos=np.c_[x, y], parent=view.scene)

app.run()