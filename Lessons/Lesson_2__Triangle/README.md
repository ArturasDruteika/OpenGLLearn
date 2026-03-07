# Lesson 2 - Triangle 

This lesson is adding a few concepts into one. I did not want to separate these concepts into 2 or more lessons lessons due to a 1 major factor. Using the newest OpenGL version (ATM, the newest is 4.6), we cannot create a triangle without using the shaders (at least we cannot do it on Linux). For this, in this lesson we are going to discuss and learn about these topics:

1. Triangle
2. Vertex Buffer Object
3. Vertex Array Object
4. Vertex / Fragment shaders

Keep in mind that these are one of the most importatn building blocks of almost all the rendering in OpenGL. 

At the end of this lesson we will learn how to render a single triangle on the GLFW window.

![Triangle](Assets/triangle.png)

---
### Triangle

#### Triangle in Rendering

So (I would hope) that all of the people are familiar of what is a simple tringle. It is a fundamental 2D shape (polygon) that has 3 straight lines, 3 vertices and 3 interior angles that always sum up to 180 degrees (PI rad) (I definately did not copy this from google). In terms of math, it is probably it (again, I am not a math major or what, maybe some mathematicians would start throwing stuff at me for describing a triangle like that). BUT, in rendering, triangle has some more properties. In rendering a triangle can have:

* Vertices
* Each vertex can have a color
* Normals
* And other stuff (which currently I do not even know)

I hope that every one of of you knows what is a vertex, but what are the other 2?

Each vertex (for a single triangle) can have a color (defined in RGB or RGBA formats (A in RGBA is basically an alpha parameter telling how opaque the RGB should look like)).

Normal is a perpendicular vector that going from the triangle plane (the plane that the triangle lives in). This is a bit harder to understand but I hope this image helps you to understand the notion of it better.

![Normal](Assets/normal_visualization.png)

#### Triangle Data

Since we know what is triangle in rendering world, a logical step frward would be to try to replicate that in the programming world. 

```C++

    float triangleVertices[] = { 0.0f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

```
This is what triangle (with colors) looks like in terms of data only. Interesting, isn't it? I mean without comments, specifically telling what these numbers mean, it would be like looking into some random array in C++. This piece of code could easily be rewritten as following:

```C++
    float triangleVertices[] = {
        // positions         // colors
         0.0f,  0.5f, 0.0f,  0.0f, 1.0f, 1.0f, 1.0f, // top vertex 
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 1.0f, 1.0f, // bottom left
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 1.0f, 1.0f  // bottom right
    };
```
Is it clearer now? Right now, it gives some instructions and rules on what these numbers mean. Going deep
