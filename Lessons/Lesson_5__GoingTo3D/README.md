# Lesson 5 - Lesson_5__GoingTo3D

This lesson is going to be an intro to a 3D renderer. But please, do not get too excited, as we are only going to discuss the basics of 3D world and we will do a simple transition from rendering 2D shapes to 3D. 

At the end of the lesson, we will be able to render a pyramid as seen below:

![pyramid](Assets/pyramid.png)

To be honest, after this lesson, you will be able to render any 3D shape, given that you correctly construct it's data.

In this lesson, we are going to cover:

* Transition from 2D to 3D world
* Intro into camera and aspect

What we are not going to cover in this lesson are:

* Interactive camera
* Live transformations (like orbiting bodies or etc.)
* Light

These topics (especially anything that has to do with lights) will be left for the following lessons. This lesson is not going to be long as the previous one. Transformations were the thing that takes time to understand and that's totally fine. Transformations, especially for those that do not like math, is a bit tricky to grasp the intuition. But this lesson is way simpler.

So, without further ado, let's continue with the lesson.


---
### Transition from 2D to 3D world

To be fair, going from 2D to 3D is kind of simple. I am not joking. The only thing that changes in the data that is being loaded to a GPU VRAM is the coordinates component (position). In 2D we had 2 numbers describing where in space the vertex should be placed (X and Y coords). In 3D world, another dimension is needed, which is called the Z dimension. Because of it, instead of 2 numbers, we need 3 numbers to describe the vertex position (X, Y and Z coords). 