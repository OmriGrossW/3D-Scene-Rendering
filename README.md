# 3D-Scene-Rendering

3D Scene Rendering - reading from obj. file.

Control Keys Summary:
  'c' for camera mode.
  's' for scene mode.
  'o' for object mode.
  '1' Camera-frame minor mode.
  '2' Scene-frame minor mode.
  '3' Object-frame minor mode.
  Use mouse according to each mode specs.


The program reads scene.obj and create a list of objects.
colorTable.csv defines ambient, diffuse and specular color properties of each
object. The first number in each row in colorTable.csv is object name and after it 3 ambient
values, 3 diffuse value and 3 specular values (RGB). For example:
1,0.5,0.4,0.3,0.2,0.9,0.8,0.7,0.6,0.1

Camera mode (the default state): When left mouse button is pressed rotate
camera around its up or right axis (rotation when camera is located at (0,0,0)) according to mouse movement and the window size.
When right mouse button is pressed zoom in when the mouse moving up and
zoom out when the mouse moving down. When middle mouse button is
pressed move the camera left and right according to mouse movement.

Scene mode: Rotation and translation should be carried according to the
camera frame of reference, that is, the scene should be rotated around it's
center, but the rotation axes are parallel to the rotation axes of the camera.
For example, by moving the mouse from left to right horizontally while in
scene mode, will result in a rotation of the scene around its center but the
rotation axis should be parallel to vertical lines on the screen.

Object mode: An object is selected by left clicking on any of the pixels belonging to it. The object
stays selected until the user selects another object, the user left click on a
location outside any other object, or the user exits object mode. By default,
no object is selected.

*When an object is selected, a red sphere with radius 0.3 should appear
around its center of mass, but it should not be occluded by any object.

The transformation behavior of the selected object is determined by 3
minor modes:

Camera-frame minor mode (default) which becomes active by
pressing '1' on the keyboard. In this mode rotation and translation
are carried according to the camera's axis system.

Scene-frame minor mode, which becomes active by pressing '2'
on the keyboard. In this mode rotation and translation are carried
according to the scene's axis system. For example, rotating around
the y axis will result in a rotation around an axis parallel to that of
the green y axis drawn on screen.

Object-frame minor mode, which becomes active by pressing '3'
on the keyboard. In this mode rotation and translation are carried
according to the object's axis system.


Texture:
Each line in the color table file can now be followed by an additional field, a
file name of a bitmap texture image. When a texture is defined for an object,
each of its faces should be mapped to the texture as follows:
  a. In the case of a triangle, the texture mapping coordinates will be
  (0,0), (0,1), (1,1) in the given order
  b. In the case of a quad, the texture mapping coordinates will be
  (0,0), (0,1), (1,1), (1,0) in the given order.
  You can assume objects with faces containing more than 4 vertices
  will not have a texture attached to them.
