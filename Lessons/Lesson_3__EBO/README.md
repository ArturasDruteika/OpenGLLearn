# Lesson 3 - Element Buffer Object

This lesson is going to be way smaller than the previous one. Mainly, because the code is 95% the same. The only thing I changed is that I added __Element Buffer Object__. Let's learn what this pbject is and why should we use it.

### What is it? Why do should we use it?

Well, this object is not making anything easier for, but rather it is helping the hardware, specifically, the VRAM, to deal with less data. How does it do it?

To answer this question, let's go back to the original data, we had in the lesson 2:

```C++
float triangleVertices[] = {
     // positions   // colors
     0.0f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top vertex 
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // bottom right
};
```

This is the data needed to render a single triangle with a color specified. How would we render 2 triangles? The first idea that come to mind is this "

```C++
float triangleVertices[] = {
     // positions   // colors
     0.0f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top vertex 
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // bottom right

     // positions   // colors
     0.0f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top vertex 
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // bottom right
};
```

At first, it looks pretty standart: just add more data describing a new triangle to the `triangleVertices`. Following this rule (18 numbers, going sequentially, in the `triangleVertices` define a single triangle) 2 triangles have 36 numbers, 3 triangles have 54 numbers and etc. The rule for this is N * 18, where N is the number of triangles.

This rule is pretty simple, but now, let's imagine a scenario where 2 triangles form a single square. This would make `triangleVertices` look like this:

```C++
float squareVertices[] = {
    // positions   // colors
    -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, // top left        triangle 1
    -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, // bottom left     triangle 1
     0.5f, -0.5f,  1.0f, 1.0f, 1.0f, // bottom right    triangle 1

    -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, // top left        triangle 2
     0.5f, -0.5f,  1.0f, 1.0f, 1.0f, // bottom right    triangle 2
     0.5f,  0.5f,  1.0f, 1.0f, 1.0f  // top right       triangle 2
};
```

Again, it looks pretty understandabe, BUT, here lies 1 problem that scales up pretty fast if the triangle number is also increasing. 

What is this problem, I want you to understand? Take a look again at the `squareVertices`. Can you see that some vertices have literally the same data? Specifically, __top left__ and __bottom right__ vertices of both triangles have exactly the same values. Look at this image:

![Same Vertices](Assets/square__same_vertices.png)

Red arrows point to the vertex for the triangle 1, green - triangle 2 (black line was added by me to visually help you to distinguish what is triangle 1 and triangle 2). What happens in 3 dimensions (what we certainly will cover in the future), the square becomes a cube. To form a cube, you need 2 x 6 = 12 triangles (cube has 6 faces, where each face is just a square, and we know that to render a square we need 2 triangles). For cubes the situation is even worse. Each cube vertex is the intersection of 3 cube faces, that means that 1 vertex is intersection of 6 triangles. With this configuation, there will be 6 defined vertex data points which are totaly the same. 

Keep in mind, that each data point (defined vertex data) takes up a real part on the GPU RAM (VRAM). Even though, 1 number takes a very tiny part on the VRAM, it still adds up to huge dublicated data if we render multiple objects every render iteration.

What I want you to understand is that you should ask yourself this "do I really need to use this simple, yet archaic method to define vertices data?". I mean since I am doing this lesson, the answer is definetly yes, there is. And this is where the __element buffer object__ comes in handy.

__Element Buffer Object (EBO)__ was created to solve this particular that I have described earlier. Using EBO, you can basically just tell which vertex should be taken for which triangle. Let's go straight to the example. Instead of this:

```C++
float squareVertices[] = {
    // positions   // colors
    -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, // top left        triangle 1
    -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, // bottom left     triangle 1
     0.5f, -0.5f,  1.0f, 1.0f, 1.0f, // bottom right    triangle 1

    -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, // top left        triangle 2
     0.5f, -0.5f,  1.0f, 1.0f, 1.0f, // bottom right    triangle 2
     0.5f,  0.5f,  1.0f, 1.0f, 1.0f  // top right       triangle 2
};
```

why can't we rewrite this as:

```C++
float triangleVertices[] = {
    // positions    // colors
    -0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top left
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom right
     0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // top right
};

unsigned int indices[] = { 
    0, 1, 2, // triangle 1
    2, 3, 0  // triangle 2
};
```
