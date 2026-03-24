# Lesson 4 - Transformations

In this tesson we are going to learn tranformations and, as a side thing, shader uniforms. Transformations, as a whole is one of those building blocks, that someone, who is learning graphics, should definitely master it. But do not get too scared, unless you want to become a savant 3D graphics engineer, transformations (at least to what I have used) is not a thing that a simple person, who does not understand math well, cannot understand.

Since I am not the best at explaining math topics and before continuing any further, I highly suggest to you to:

* Read [this article](https://learnopengl.com/Getting-started/Transformations)
* If you are more of a visual learning, I advise you to look [this playlist](https://www.youtube.com/watch?v=fNk_zzaMoSs&list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab) by [3Blue1Brown](https://www.youtube.com/@3blue1brown/playlists). This, in my opinion, is the best way to visually understand the concepts of linear algebra.

Again, do not get discouraged by the fact than N years ago, during math classes you did not pay attention, because learning transformations is a process, where each person, given time, can understand.

As for the uniforms, it is also, one of the building blocks you have to understand, but it is as simple as learning what a shader or vertex buffer is.

---

### Essence of Transformations in Rendering

I will try to give my own personal view, on why the tranformations are one of the building blocks in rendering. 

One important thing you have to understand about building a graphics engine is that no everything inside the graphics engine revolves around rendering. For example, let's say you want to render a trinagle. From the previous lessons, we know that a triangle is comprised of 3 vertices. In order for the OpenGL to see how you have defined those 3 vertices, you need to upload that that vertex data array to GPU using `glBufferData`. Now stop right here, before the `glBufferData` operation is called. Let's focus purely on the `triangleVertices` (from the previous lesson) array.

If we isolated this `triangleVertices` array from all the OpenGL operations, could we say that `triangleVertices` is somehow related to rendering? Just look here:

```C++
int main()
{
    // Some code

    float triangleVertices[] = {
        // positions    // colors
         0.0f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top vertex 
        -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom left
         0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // bottom right
    };

    // Some code

    return 0;
}

```

Looking at this piece of code, does it give any information that we are going to render a triangle on the screen defined by `triangleVertices`? No, it does not anbd this is the one of the major separation of concerns you have to understand. The whole graphics project you are building can (and matter of fact should be) be separated into 2 parts:

1. __Rendering__ - everything regarding graphics api for OpenGL, Vulkan or any other graphics backend.
2. __Real World__ - everything regarding the real world logic, like geometrical objects definitions, geometrical objects construction, physics, lighting, orientations, scenes, etc.

What rendering is, you probobly know already, but what a real world it is this new concept that at first might be weird to understand as to why do we even need to separate it from the rendering. To understand easier what __real world__ I suggest changing the "real" with "virtual", thus __real world__ becomes __virtual world__. Does it make sense now? Every object we create, every logic we apply to it only lives inside computers, thus making it virtual. In order to render something, that something needs to be described and built from scratch. Going back to the triangle example, we have to provide OpenGL some data that it has to render. This "data" is what I (and I hope a lot of other people) call __real world__. This data for the triangle is quite simple, just 18 numbers in a sequential order. But in order to render something that would visually awe other people, a simple triangle is not enough. Just imagine, how many different geometrical shapes are required to render this frame:

![Ghost Of Tsushima Real World](Assets/ghost_of_tsushima_real_world.png)

Just look at this frame, this scene has so many object, like:

* 3 people + 1 laying dead
* Different clothing + different inventory (katanas, wakizashi, etc.)
* Environment, with trees, leaves, grass, rocks, mud, etc.
* Lighting and shadows (these 2 things are very big in terms of code lines)

What this can and cannot show in a single frame is the "flow" of the scene. I mean this is just a single scene, so no movement, either for characters, or for environmental things, cannot be seen, but you can almost feel, how this scene is playing out in you head. This movement is also a separate aspect in real world. 

Real world, most of the times, take up way more lines of code, than just calling some graphics backend API to render it. From my own personal experience, when creating my Andromeda graphics engine, I noticed that I spend 10x more time perfecting how the real world works (object creation, object movement, object transformations) than just simple writing code for how to render that real world. I, distictly, remember, how I came to a realization that you need to separate what rendering and real world is. It kinda scared me at the beginning, but now I am really glad that I have created my own small virtual reality which I can show to other people.

I am telling you this, because real world without transformations does not work. No matter how you want to bypass transformations, it is impossible. Without transformations, there is no way to smoothly show how movement works, it is impossible to implement illumination with realistic shadows. That is why I decided that lesson 4 is going to be about transformations.

Of course, this lesson is not going to be a full walkthrough on how to render scenes like the image I showed you before. I would have to write nonstop for like 2 weeks (no breaks just constant writing) to be able to tell you how to do stuff like that. I myself, at least at this point in my time, would not be able to creating games likes this, because it is really hard and takes enormous amount of time to make it happen. Just imagine, this game "ghost of tsushima" (the image I shared), was being created for 4 - 6 years and by ~150 people (I cannot prove the exact numbers, it was what I could find on the internet, so the numbers could be different). Imagine, the amount of total hours it takes to do this kinda stuff. And trust me when I say this, but it is impossible to create a game like this (or many other games) without transformations.

As I said before, rendering, in my opinion, without transformations is not rendering at all. Why? Because without transformations, it would be extremely hard to show something that is ___changing___. For example, imagine that you want to render a basketball going into a basket from a 3 pt line. Without transformations, the only way to smoothly render this animation, would be to create a new sphere for the basketball every render iteration with different values for coordinates. That is as ineficiant as it can get. Now imagine 100 basketballs going to the hoop at the same time, I think you get the gist of it. Another example would be zooming the view (we are not going to cover this now, but to do it you also need to use transformations). Without transformations, every zoom you do, you have to create a new object, that has a bigger size. Imagine that you are zooming in on a circle. Every zoom a new larger sphere has to be created.

With transformations, instead of creating new objects, why can't we just change the way they appear on our screen. This is many times faster and "healthier" for computers to handle. Why? To answer this, just for a second, forget that there are transformations. Without them, if our rendered scene changes even the slightest, we would have to recreate all of our scene from scratch. That would mean that we have to:

1. Create data buffers
2. Reuse rules on how to read this data buffer
3. Delete previously used objects
4. Construct new objects
5. Etc.

This is extremely cost ineffective. Computers are fast, but that definitely hinders their speed by a lot.

With transformations, instead of doing all those steps, why can't we just manipulate the data we already have in our data buffer? Let's take our good old triangle data from previous lessons:

```C++
float triangleVertices[] = {
    // positions    // colors
    0.0f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top vertex 
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom left
    0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // bottom right
};
```

If we want to change the vertex position (`0.0, 0.5`), without transformations, we would have to delete this buffer and create the new one. With transformations, we can directly access the data buffer and according to some translation rule, we update the XY coords of the first vertex.

To me, that is the essence of transofrmations in rendering. You just have some data in the buffer and you manipulate it according to your needs or rules that you have defined.

---
### Transformation types

For the basic 3D object transformation, in my opinion, you need to know 3 types of transformations:

1. __Scaling__ - resizing the object to make it bigger or smaller.
![Scaling](Assets/scaling.png)
2. __Rotation__ - rotating the object around one of it's axis.
![Rotation](Assets/rotation.png)
3. __Translation__ - moving an object in space from position A to position B.
![Translation](Assets/translation.png)

This is probably 99% of transformations you will need if you want to understand how objects manipulation works. Again, I just want to stress this that this topic of __transformations__ is also an integral part if you want to understand how rendering works. I know, that it may sound weird, I mean "how does knowing how objects can be transformed help me to understand how to render an image". But without tranformations, all of your scenes will be just a static image where nothing happens. 

Keep in mind, that I just showed you how transformations happen in 2D. But do not get too afraid, in 3D, these transformations happen the same way, just that there are more axis on which we can transform objects.

Also, an important thing to say is that these 3 transformation are ony for the objects manipulations. These transformations only change the objects in world space (we are going to talk about this in this lesson later). Without these 3 matrix transformations, there are others, like __view__, __projection__ transformations. But, these topics are going to be left for the following lessons, because those topics are a bit harder to explain.


### Transformations

Since you read (I hope you did) [this article](https://learnopengl.com/Getting-started/Transformations), I can move directly to explaining how I understand what transformations is, what they are and how to use them. Our end goal is this window:

![triangle_transformations](Assets/triangle_transformations.png)

In this image you can see 4 different triangles placed on 4 different screen positions. First of a all, what do these triangles simbolize:

1. ___Top Left Corner___ - triangle that has 0 transformations applied.
2. ___Bottom Left Corner___ - triangle that has __rotation__ transformation applied.
3. ___Top Right Corner___ - triangle that has __scaling__ transformation applied.
4. ___Bottom Right Corner___ - triangle that has __rotation__ and __scaling__ transformations applied.

Before explaining how did I tranform these triangles, I want to say that there is another, "secret" transformation that has been applied to all of these 4 triangles. It is the __translation__ transformation that has been applied to all of these 4 triangles.

Also, from those tutorials you have read (that I hope you read), I hope you remember that there many coordinate systems for objects and other "stuff" like:

* Model space
* World space
* View space
* Projection space
* Clip space

In this lesson, we are only going to cover __local space__ and __model space__ (sometime in the future, I will for sure cover the remaining coordinates systems). We need to understand the first and the second coordinate systems to understand how transformations happen.

One additional important fact I want to share with you. Since we are talking about transformations for the objects, from now on I want you to separate one thing. As I said in previous lessons, rendering is generating pixel color values, that can be shown on the screen, but there is one important concept you have to know. It is that there are 2 things you have to constantly work with, and they are:

1. Purely rendering part, where given the data, computer generates pixel colors, based on the rendering pipeline you constructed.
2. Real world part.

The first thing is what we have learned in the first 3 lessons. The second one is where you will spend a lot of time also.


#### Model Space

__Model space__ is a type of coordinate system that is relative to object itself. That means that the object is centered around the origin (if it is 2D then it is [ 0, 0 ], if it is 3D - [ 0, 0, 0 ], etc.). Why do we need this type of coordinate system? Well, it is great for describing a single object. As alwasy, an example is worth more than a million words.

So imagine you want to create a triangle


### Code Part

Before I show you the code for this lesson, I want inform you that the code from the previous lesson is changed here. The things I have changed are:

1. I have moved the shaders code in separate files. I did that, because this is the convention on how to handle shaders (at least in our situations). For this and many more following lessons, we are not going to do shader generations throughout the lifetime of a program (that is what we maybe do far far into the future).
2. Added my own library from [my own grahics engine](https://github.com/ArturasDruteika/Andromeda). For the lessons, I will be naming it __Orion__. So everything, that you can find in the __Orion__ folder, is what I use when I am creating __Andromeda__ graphics engine.
3. Added __glm__ 3rd party. This is a great library for math. It has all the needed transformations and etc. when working with OpenGL.

Also, as a side note, from this point, moving in the future, we will be refactoring a lot of our code, to make it follow best practices. I am not saying, that all of our code will be the best it can, absolutely no, because then it will be hard to explain everything. But we will stop flooding our main.cpp file with all the code for a single lesson. Instead, we will separate what can be separated into classes, files and etc., so do not be scared.

Without any further pointless talks, let's dive deep into the code part, where I will show you how to code the transformation and how to use uniforms using OpenGL.


#### Uniforms

First, let's start here, since it will be over in a half a minute. Uniform is a way that let's developers to directly pass variables to shaders. What is the point of allowing developers directly pass values to the shaders? The first thing that comes to my mind is the paralellism of GPU. Remember what is the definition of a __shader__? (cough cough, a program that runs on GPU)... Also, remember that GPUs are great for program executions that are independent from one another (you can google what SIMD is). 

Ok, I will elaborate on this a little more. ___The following example is going to be run on the CPU, meaning a single core of CPU is going to be used___. Immagine a simple array of numbers, let's define it as `int arrayOfNumbers[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};`. Imagine that there is a function that takes exactly 1s to complete, which does: 

```C++
int Some1SFunction(int originalValue):
    ...
    return processedOuput;
```

Also, imagine, that now, I want to apply this function to all of the numbers in the `arrayOfNumbers`. One way to do this is the following:

```C++
for (const number : arrayOfNumbers)
    int result = Some1SFunction(number);
```

On CPU, this function would take 9s to complete, since it would be called 9 times. To speed this up, we can use threads. So if our CPU had 9 or more cores, each core could take a single call of `Some1SFunction` and voila, the time was reduced from 9s to just 1s.

Ok, but what if our array has 10000 numbers? The CPU definitely does not have cores in terms of thousands. That's sad, but you know what has it? Yes, GPU. With some programming knowledge, you could run this operation on that 10000 number array, and since GPUs have thousands of minicores, this operation could be done seconds.

Where do uniforms come into play? Well, remember that most of the time, we have arrays of vertex data. This array can have thousands of vertices defined. Imagine, that we wanted to multiple each vertex position by some matrix (which we are going to talk in the transformations section). Each of those transformations on each vertex will definitely take a toll on the speed of the rendering. This is where __uniforms__ become handy. Let's take vertex shader for example. It can process many vertices at the same time because the VS is running on the GPU. Because of it, we can apply that matrix to each of the vertices many times faster than it could be done on the CPU. But, how to tell the GPU which matrix to apply? I mean GPU does not have in it's own memory no notion of that matrix you want to apply to each vertex. To bypass it, you can add a changable parameter to the shader program called __uniform__.

Take a look:

```C++
#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

uniform mat3 u_transform;

out vec4 ourColor;

void main()
{
    vec3 transformed = u_transform * vec3(aPos, 1.0);
    gl_Position = vec4(transformed.xy, 0.0, 1.0);
    ourColor = aColor;
}
```

`uniform mat3 u_transform;` is the newly added. Now the GPU knows, that there is an additional parameter, that is going to controlled by the user him/herself. We have a direct access from the program side to set a 3x3 size matrix to this shader. 

Just remember one thing, that uniforms allow developers to directly set values in the shader programs.


