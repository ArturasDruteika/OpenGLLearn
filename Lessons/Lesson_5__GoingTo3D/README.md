# Lesson 5 - Lesson_5__GoingTo3D

This lesson is going to be an intro to a 3D renderer. But please, do not get too excited, as we are only going to discuss the basics of 3D world and we will do a simple transition from rendering 2D shapes to 3D. 

At the end of the lesson, we will be able to render a pyramid as seen below:

![pyramid](Assets/pyramid.png)

To be honest, after this lesson, you will be able to render any 3D shape, given that you correctly construct it's data.

In this lesson, we are going to cover:

* Transition from 2D to 3D world
* Intro into camera 
* Aspect ratio

What we are not going to cover in this lesson are:

* Interactive camera
* Live transformations (like orbiting bodies or etc.)
* Light

These topics (especially anything that has to do with lights) will be left for the following lessons. This lesson is not going to be long as the previous one. Transformations were the thing that takes time to understand and that's totally fine. Transformations, especially for those that do not like math, is a bit tricky to grasp the intuition. But this lesson is way simpler.

So, without further ado, let's continue with the lesson.


---
### Transition from 2D to 3D world

To be fair, going from 2D to 3D is kind of simple. I am not joking. The only thing that changes in the data that is being loaded to a GPU VRAM is the coordinates component (position). In 2D we had 2 numbers describing where in space the vertex should be placed (X and Y coords). In 3D world, another dimension is needed, which is called the Z dimension. Because of it, instead of 2 numbers, we need 3 numbers to describe the vertex position (X, Y and Z coords). Look at this image bellow:

![2D_3D_graph](Assets/2D_3D_graph.png)

Is it clearer now? 

One thing to rememeber (or you can think about it as guidlines) is the axis directions. By convention:

* X axis: left <--> right
* Y axis: up <--> down
* Z axis: depth

#### Z axis

X and Y axes are pretty simple to understand, but Z is a bit trickier. You can think of this depth dimension the same way depth is measured in water. Imagine, that you put your face paraller to the water surface, so that your eyes would look directly into the water. This way the depth of the water acts as a Z dimension.

Also, another important thing is the Z axis convention. There are 2 way how you can measure an axis:

1) Right hand rule: the Z axis is poisitve going towards you (in rendering it would be towards the screen) (this is the convention)
2) Left hand rule: the Z axis is negative going away from you (in rendering it would be away from the screen)

This graph below clearly demonstrates this:

![opengl_3D_coords](Assets/opengl_3D_coords.jpeg)

To be honest, I do not know what else to talk about 3D real world. The main thing is that coordinates expand to another dimension (Z axis).


---
# Camera

For me, the best way to think about camera is like an actual operator is filming the scene. Imagine, that you have a scene which has some objects placed in it (like tables, chairs and etc.). Now the operator can film the scene from many positions, maybe from the left side, maybe from the right. The operator can film the scene from the bottom or the top side. In other words, the operator can film it from any direction. The camera produces the visual information (what it captured), thus allowing viewers to see what is happening in that scene. The different position of the camera, produces the different view of the scene.

The exact same principles apply for the camera term in rendering. Instead of a human carying that camera from place to place, it is controlled by the user