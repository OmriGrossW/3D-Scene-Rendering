# 3D-Scene-Rendering

3D Scene Rendering - reading from obj. file.

ssssss

The program reads scene.obj and create a list of objects.
colorTable.csv defines ambient, diffuse and specular color properties of each
object. The first number in each row in colorTable.csv is object name and after it 3 ambient
values, 3 diffuse value and 3 specular values (RGB). For example:
1,0.5,0.4,0.3,0.2,0.9,0.8,0.7,0.6,0.1

Camera mode (the initial state): When left mouse button is pressed rotate
camera around its up or right axis (rotation when camera is located at (0,0,0))according to mouse movement and the window size (see explanation below).
When right mouse button is pressed zoom in when the mouse moving up and
zoom out when the mouse moving down. When middle mouse button is
pressed move the camera left and right according to mouse movement.

