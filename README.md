# 5607finalProject

TODO:

No way to input a function, they currently have to be hardcoded

Lighting is not especially nice, especially with flat U and V values

Some implementation of GL_Lines primitive, for axis, tick marks, potentially grides, lines along functions
  Could look at animating some things with these

Blendshape type transformation between functions? linear interpolation in z

potential alpha channel use to look at multiple surfaces at once

Animation of balls on surfaces? (simple implementation just send them in the direction of greatest desent)

Fractals / recursive functions (Time to build model? Might not be able to be dynamic)

Some kind of GUI

Rotation doesn't work properly (especially left right when
  not looking from even with the horizon)



OPTIMIZATIONS:
Less pointers in model creation
MultiThread model creation? PragmaOMP?
Textures and UV coords can probably be removed, save a lot of space?
Occasiaonally trim down degree of polynomials to save on iterations
