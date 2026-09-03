# Lesson 4 — Transformations

In the previous lessons, we learned how to describe geometry, upload vertex data to the GPU, configure vertex attributes, use shaders, and render indexed geometry.

So far, however, our objects have mostly remained exactly where their vertex data placed them.

In this lesson, we are going to change that.

We will introduce **transformations**, which allow us to:

- move objects
- rotate objects
- resize objects
- combine multiple transformations
- place objects into a shared world
- reuse the same geometry at different locations

As a related topic, we will also introduce **shader uniforms**, which allow our C++ program to provide values such as transformation matrices to shaders.

By the end of this lesson, you should understand:

1. Why transformations are important in graphics.
2. The three basic object transformations: translation, rotation, and scaling.
3. The difference between model space and world space.
4. Why matrices are used to represent transformations.
5. Why homogeneous coordinates are necessary for translation.
6. What a model matrix is.
7. Why transformation order matters.
8. What shader uniforms are.
9. How to send a transformation matrix from C++ to a vertex shader.
10. How to render the same geometry using different transformations.

---

# Contents

1. [Why Transformations Matter](#1-why-transformations-matter)
2. [The Three Basic Transformations](#2-the-three-basic-transformations)
   - [2.1 Translation](#21-translation)
   - [2.2 Rotation](#22-rotation)
   - [2.3 Scaling](#23-scaling)
3. [Our Goal](#3-our-goal)
4. [Coordinate Spaces](#4-coordinate-spaces)
   - [4.1 Model Space](#41-model-space)
   - [4.2 World Space](#42-world-space)
   - [4.3 From Model Space to World Space](#43-from-model-space-to-world-space)
5. [Why We Use Matrices](#5-why-we-use-matrices)
   - [5.1 Combining Transformations](#51-combining-transformations)
   - [5.2 The Identity Matrix](#52-the-identity-matrix)
6. [Homogeneous Coordinates](#6-homogeneous-coordinates)
   - [6.1 Why a 2x2 Matrix Cannot Translate a Point](#61-why-a-2x2-matrix-cannot-translate-a-point)
   - [6.2 Representing 2D Points with Three Components](#62-representing-2d-points-with-three-components)
   - [6.3 Translation Matrix](#63-translation-matrix)
   - [6.4 Rotation Matrix](#64-rotation-matrix)
   - [6.5 Scaling Matrix](#65-scaling-matrix)
   - [6.6 The Meaning of W](#66-the-meaning-of-w)
7. [The Model Matrix](#7-the-model-matrix)
   - [7.1 Transformation Order](#71-transformation-order)
8. [Shader Uniforms](#8-shader-uniforms)
   - [8.1 Adding a Uniform to the Vertex Shader](#81-adding-a-uniform-to-the-vertex-shader)
   - [8.2 Uniform Values and Draw Calls](#82-uniform-values-and-draw-calls)
9. [Using GLM](#9-using-glm)
   - [9.1 Creating a Translation Matrix](#91-creating-a-translation-matrix)
   - [9.2 Creating a Rotation Matrix](#92-creating-a-rotation-matrix)
   - [9.3 Creating a Scaling Matrix](#93-creating-a-scaling-matrix)
10. [Creating the Transformations](#10-creating-the-transformations)
11. [Sending a Matrix to the Shader](#11-sending-a-matrix-to-the-shader)
   - [11.1 glGetUniformLocation](#111-glgetuniformlocation)
   - [11.2 glUniformMatrix3fv](#112-gluniformmatrix3fv)
12. [Rendering Multiple Transformed Objects](#12-rendering-multiple-transformed-objects)
13. [Common Transformation Mistakes](#13-common-transformation-mistakes)
14. [Conclusion](#14-conclusion)

---

# Recommended Resources

Transformations rely heavily on vectors, matrices, and basic linear algebra.

You do not need to become a mathematician to use them effectively, but understanding the underlying concepts will make graphics programming considerably easier.

If any of the mathematics in this lesson feels unfamiliar, these resources are highly recommended:

- [LearnOpenGL — Transformations](https://learnopengl.com/Getting-started/Transformations)
- [3Blue1Brown — Essence of Linear Algebra](https://www.youtube.com/watch?v=fNk_zzaMoSs&list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab)
- [3Blue1Brown](https://www.youtube.com/@3blue1brown/playlists)

For rotation matrices specifically:

- [Rotation Matrices — Dr. Trefor Bazett](https://www.youtube.com/watch?v=rUKsjo1nReE)
- [Rotation Matrix — Dr Peyam](https://www.youtube.com/watch?v=Ta8cKqltPfU&t=380s)

Do not worry if matrices initially feel unfamiliar.

The important ideas become much easier once you begin using them in actual graphics code.

---

# 1. Why Transformations Matter

One of the most important ideas in graphics programming is that **geometry and its placement do not have to be the same thing**.

Consider the triangle from the previous lessons:

```cpp
float triangle_vertices[] = {
    // positions    // colors
     0.0f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // bottom right
};
```

This array describes the triangle's geometry.

By itself, however, the array does not say:

- which graphics API will render it
- where the object belongs in a larger scene
- whether the object should move
- whether it should rotate
- whether it should become larger or smaller

It is simply data.

A useful way to structure a graphics application is to think about two broad responsibilities:

    Application / Scene
        ↓
    objects
    positions
    orientations
    scales
    animation
    physics
    lights
    cameras
    scene relationships

and:

    Rendering
        ↓
    OpenGL
    shaders
    buffers
    textures
    draw calls
    GPU state

These systems interact, but they are not the same thing.

For example, your game or engine may determine:

> The basketball should now be at position `(3.2, 5.7)`.

The renderer's job is then to display the basketball at that position.

We should not need a completely different mesh for every possible location of the basketball.

Instead, we can create the basketball's geometry once and then **transform** that geometry when rendering it.

---

## Transforming Instead of Rebuilding Geometry

Suppose our triangle is centered around its own local origin.

If we want another triangle somewhere else, one approach would be to manually change every vertex position.

For example:

```cpp
float triangle_vertices[] = {
    // manually shifted 4 units to the left
     0.0f - 4.0f,  0.5f,
    -0.5f - 4.0f, -0.5f,
     0.5f - 4.0f, -0.5f
};
```

This works mathematically, but it mixes two different ideas:

1. What does the triangle look like?
2. Where should the triangle be placed?

A much better approach is:

    Define the triangle once
        ↓
    keep its vertices in local coordinates
        ↓
    apply a transformation
        ↓
    place it wherever we want

For example:

    Original triangle
        ↓
    translation of (-4, 0)
        ↓
    same triangle, moved 4 units left

This allows the original geometry to remain unchanged.

> **Transformations allow us to separate an object's geometry from how that object is positioned, oriented, and scaled in a scene.**

---

## Do We Have to Recreate Buffers Without Transformations?

No.

It is important to make this distinction.

OpenGL provides several ways to modify vertex-buffer data after creating a buffer, including functions such as:

```cpp
glBufferSubData(...)
```

and other buffer-update techniques.

So transformations are **not** useful because changing a position would otherwise require deleting and recreating a buffer.

Their real advantage is architectural and computational.

Instead of modifying potentially thousands of vertex positions on the CPU whenever an object moves, we can keep the object's local geometry unchanged and provide a transformation to the vertex shader.

Conceptually:

    Original vertex
        ↓
    vertex shader
        ↓
    transformation matrix
        ↓
    transformed vertex

The same geometry can therefore be rendered:

- at different positions
- at different rotations
- at different sizes

without rewriting the original vertex data.

---

## Example: Moving a Basketball

Imagine a basketball moving toward a hoop.

We might have:

1. A sphere mesh representing the basketball.
2. A hoop mesh.
3. Physics or animation logic calculating the ball's position over time.

The basketball mesh does not need to be reconstructed every frame.

Instead:

    basketball mesh
        +
    current position
        ↓
    model transformation
        ↓
    render basketball at that position

As the ball moves:

    Frame 1 → position A
    Frame 2 → position B
    Frame 3 → position C
    Frame 4 → position D

The geometry remains the same.

Only its transformation changes.

This concept applies to practically every moving object in a 3D scene.

![Ghost Of Tsushima Real World](assets/ghost_of_tsushima_real_world.png)

A scene such as the one above may contain:

- characters
- weapons
- clothing
- trees
- grass
- rocks
- terrain
- particles
- lights
- shadows
- animated objects

Each object may have its own position, orientation, and scale.

Transformations are one of the fundamental tools that make such scenes possible.

---

# 2. The Three Basic Transformations

For basic object manipulation, there are three transformations that we need to understand:

1. **Translation**
2. **Rotation**
3. **Scaling**

These concepts work in both 2D and 3D.

The main difference is simply the number of axes involved.

---

## 2.1 Translation

**Translation** moves an object from one location to another.

![Translation](assets/translation.png)

In 2D, a translation has two components:

    tx → movement along the X axis
    ty → movement along the Y axis

For example:

    translation = (3, -2)

means:

    move 3 units along X
    move -2 units along Y

If a point starts at:

    (4, 7)

then applying the translation:

    (3, -2)

produces:

    (4 + 3, 7 - 2)

which gives:

    (7, 5)

Translation therefore behaves like an **offset** or **displacement**.

---

## 2.2 Rotation

**Rotation** changes an object's orientation.

![Rotation](assets/rotation.png)

In 2D, rotation happens around a point, usually the object's local origin.

For a point:

    (x, y)

rotated by an angle `θ` around the origin:

    x' = x cos(θ) - y sin(θ)
    y' = x sin(θ) + y cos(θ)

In 3D, rotation becomes more involved because an object can rotate around the:

- X axis
- Y axis
- Z axis

We will explore 3D rotations more deeply in later lessons.

---

## 2.3 Scaling

**Scaling** changes the size of an object.

![Scaling](assets/scaling.png)

In 2D, we can represent scale using two values:

    sx → scale along X
    sy → scale along Y

For example:

    scale = (2, 2)

doubles the size of the object along both axes.

A scale of:

    scale = (0.5, 1.5)

means:

    X → half the original size
    Y → one and a half times the original size

When each axis uses a different scale value, we call it **non-uniform scaling**.

When every axis uses the same value, we call it **uniform scaling**.

---

# 3. Our Goal

The goal of this lesson is to render four copies of the same triangle.

![Triangle Transformations](assets/triangle_transformations.png)

Each triangle will use the same underlying geometry but a different transformation.

The four triangles will demonstrate:

1. **Top-left**
   - translation

2. **Bottom-left**
   - translation
   - rotation

3. **Top-right**
   - translation
   - scaling

4. **Bottom-right**
   - translation
   - rotation
   - scaling

Conceptually:

    Same triangle geometry
        │
        ├── translation
        │      ↓
        │   top-left
        │
        ├── translation × rotation
        │      ↓
        │   bottom-left
        │
        ├── translation × scale
        │      ↓
        │   top-right
        │
        └── translation × rotation × scale
               ↓
            bottom-right

The geometry itself does not need to change.

Only the transformation supplied to the shader changes.

---

# 4. Coordinate Spaces

Before we start constructing transformation matrices, we need to understand **coordinate spaces**.

Graphics applications commonly move vertex positions through several coordinate systems.

A simplified 3D rendering pipeline may look like:

    Model / Local Space
        ↓
    World Space
        ↓
    View / Camera Space
        ↓
    Clip Space
        ↓
    Normalized Device Coordinates
        ↓
    Screen / Window Space

In this lesson, we are primarily concerned with:

1. **Model space**
2. **World space**

View and projection transformations will be introduced in later lessons.

---

## 4.1 Model Space

**Model space**, also called **local space** or **object space**, is the coordinate system in which a model's vertices are originally defined.

For our triangle:

```cpp
float triangle_vertices[] = {
    // positions
     0.0f,  0.5f,
    -0.5f, -0.5f,
     0.5f, -0.5f
};
```

the vertex positions describe the triangle relative to its own local coordinate system.

Conceptually:

    Model Space

              y
              ↑
              |
            (0, 0.5)
             / \
            /   \
           /     \
          /       \
    (-0.5,-0.5)  (0.5,-0.5)
              |
    ----------+----------→ x
            origin

An important detail is that the object's origin does **not** have to be its geometric center.

The local origin is simply the origin chosen when the object or mesh is created.

For many simple examples, placing the geometry around `(0, 0)` is convenient because rotations and scaling naturally occur around that origin.

---

## 4.2 World Space

**World space** is a shared coordinate system used to describe where objects exist relative to one another in a scene.

Imagine that we have several objects:

    Triangle → (-4,  2)
    Square   → ( 3,  7)
    Circle   → ( 8, -2)

Each object can be defined in its own local coordinate system while also having a position in the shared world.

![Model Space Rectangle](assets/model_space_rectangle.png)

In model space, we focus on the geometry of one object.

![World Space Shapes](assets/world_space_shapes.png)

In world space, we care about how multiple objects are positioned relative to one another.

A useful summary is:

> **Model space describes an object locally. World space describes where that object belongs in the scene.**

---

## 4.3 From Model Space to World Space

The transformation that converts a point from model space into world space is normally called the **model transformation**.

Conceptually:

    model-space vertex
        ↓
    model matrix
        ↓
    world-space vertex

Mathematically:

    p_world = M × p_model

where:

    p_model = position in model space
    M       = model matrix
    p_world = resulting world-space position

The model matrix may contain:

- translation
- rotation
- scaling

or any combination of them.

> In this lesson, because we have not introduced a view or projection matrix yet, the transformed coordinates are ultimately being used directly to produce clip-space positions in the vertex shader. We are using "world space" as the conceptual scene space while learning how the model transformation works.

---

# 5. Why We Use Matrices

We could transform vertices manually.

For example, suppose we want to:

1. Scale a triangle by `2`.
2. Rotate it by `90°`.

Without matrices, we could write something similar to:

```cpp
struct Vec2
{
    float x;
    float y;
};

std::array<Vec2, 3> triangle =
{
    Vec2{  0.0f,  0.5f },
    Vec2{ -0.5f, -0.5f },
    Vec2{  0.5f, -0.5f }
};

const float scale_factor = 2.0f;

for (Vec2& vertex : triangle)
{
    // Scale
    vertex.x *= scale_factor;
    vertex.y *= scale_factor;

    // Rotate 90 degrees counter-clockwise
    const float old_x = vertex.x;
    const float old_y = vertex.y;

    vertex.x = -old_y;
    vertex.y =  old_x;
}
```

This works.

Matrices do not magically eliminate the mathematical work required to transform a vertex.

Their major advantage is that they provide a **standard representation for transformations**.

Instead of handling every transformation as unrelated code, we can represent them all using matrices.

For example:

    S → scaling matrix
    R → rotation matrix
    T → translation matrix

These matrices can then be combined.

---

## 5.1 Combining Transformations

Suppose we want:

    scale
        ↓
    rotate
        ↓
    translate

Instead of treating these as unrelated operations, we can construct one combined matrix:

    M = T × R × S

Then each vertex only needs the combined transformation:

    p_world = M × p_model

The matrix multiplication:

    T × R × S

can be calculated once whenever the object's transformation changes.

The resulting model matrix can then be reused for every vertex belonging to that object.

This is extremely useful because a mesh may contain:

    3 vertices
    300 vertices
    30,000 vertices
    3,000,000 vertices

while the object's transformation may still be represented by just one matrix.

Matrices give us several important benefits:

- transformations have a consistent mathematical representation
- multiple transformations can be composed together
- the resulting transformation can be reused
- the same concepts extend naturally from 2D to 3D
- GPUs are designed to perform vector and matrix operations efficiently
- transformation logic fits naturally into vertex shaders

---

## 5.2 The Identity Matrix

An important matrix in graphics is the **identity matrix**.

In 2D homogeneous coordinates, the identity matrix is:

$$
\begin{bmatrix}
1 & 0 & 0 \\
0 & 1 & 0 \\
0 & 0 & 1
\end{bmatrix}
$$

Multiplying a vector by the identity matrix does not change it:

$$
I\mathbf{v} = \mathbf{v}
$$

The same applies to another matrix:

$$
IM = M
$$

and:

$$
MI = M
$$

This makes the identity matrix a convenient starting point when constructing transformations.

With GLM:

```cpp
glm::mat3 matrix(1.0f);
```

creates a `3 × 3` identity matrix.

---

# 6. Homogeneous Coordinates

So far, scaling and rotation can be represented using ordinary `2 × 2` matrices.

Translation presents a problem.

To understand why, we need to introduce **homogeneous coordinates**.

---

## 6.1 Why a 2x2 Matrix Cannot Translate a Point

A general `2 × 2` matrix looks like:

$$
\begin{bmatrix}
a & b \\
c & d
\end{bmatrix}
$$

Multiplying it by:

$$
\begin{bmatrix}
x \\
y
\end{bmatrix}
$$

produces:

$$
\begin{bmatrix}
ax + by \\
cx + dy
\end{bmatrix}
$$

Every output component is built from the original `x` and `y`.

There is nowhere to introduce a constant translation such as:

$$
x + t_x
$$

or:

$$
y + t_y
$$

Another way to understand this is that ordinary linear transformations always map the origin to the origin.

If:

$$
\mathbf{v} =
\begin{bmatrix}
0 \\
0
\end{bmatrix}
$$

then for any `2 × 2` linear transformation matrix `A`:

$$
A\mathbf{v} =
\begin{bmatrix}
0 \\
0
\end{bmatrix}
$$

But translation should be able to map:

$$
\begin{bmatrix}
0 \\
0
\end{bmatrix}
$$

to:

$$
\begin{bmatrix}
t_x \\
t_y
\end{bmatrix}
$$

Translation is therefore an **affine transformation**, rather than a linear transformation in ordinary 2D coordinates.

Homogeneous coordinates allow us to represent affine transformations using matrix multiplication.

---

## 6.2 Representing 2D Points with Three Components

Instead of representing a 2D point as:

$$
\begin{bmatrix}
x \\
y
\end{bmatrix}
$$

we represent it in homogeneous coordinates as:

$$
\begin{bmatrix}
x \\
y \\
1
\end{bmatrix}
$$

We have added a third component called `w`.

For a normal 2D point in this lesson:

    w = 1

Our transformation matrix therefore changes from:

    2 × 2

to:

    3 × 3

This additional dimension gives us enough room to represent translation alongside rotation and scaling.

---

## 6.3 Translation Matrix

A 2D translation matrix is:

$$
T =
\begin{bmatrix}
1 & 0 & t_x \\
0 & 1 & t_y \\
0 & 0 & 1
\end{bmatrix}
$$

Now multiply it by a point:

$$
\begin{bmatrix}
1 & 0 & t_x \\
0 & 1 & t_y \\
0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
x \\
y \\
1
\end{bmatrix}
$$

The result is:

$$
\begin{bmatrix}
x + t_x \\
y + t_y \\
1
\end{bmatrix}
$$

That is exactly what we wanted.

The translation:

    (tx, ty)

has been incorporated into an ordinary matrix multiplication.

This is one of the major reasons homogeneous coordinates are so useful.

> **Homogeneous coordinates allow translation, rotation, and scaling to be represented within the same matrix framework.**

---

## 6.4 Rotation Matrix

The standard 2D rotation matrix is:

$$
R =
\begin{bmatrix}
\cos\theta & -\sin\theta & 0 \\
\sin\theta &  \cos\theta & 0 \\
0          &  0          & 1
\end{bmatrix}
$$

Multiplying it by:

$$
\begin{bmatrix}
x \\
y \\
1
\end{bmatrix}
$$

produces:

$$
\begin{bmatrix}
x\cos\theta - y\sin\theta \\
x\sin\theta + y\cos\theta \\
1
\end{bmatrix}
$$

For positive angles with the coordinate conventions used here, this represents counter-clockwise rotation around the origin.

---

## 6.5 Scaling Matrix

A 2D scaling matrix is:

$$
S =
\begin{bmatrix}
s_x & 0   & 0 \\
0   & s_y & 0 \\
0   & 0   & 1
\end{bmatrix}
$$

Multiplying it by a point gives:

$$
\begin{bmatrix}
s_x x \\
s_y y \\
1
\end{bmatrix}
$$

Therefore:

    sx → controls scaling along X
    sy → controls scaling along Y

---

## 6.6 The Meaning of W

The additional component in a homogeneous vector is conventionally called `w`.

A point can be represented as:

$$
\begin{bmatrix}
x \\
y \\
w
\end{bmatrix}
$$

For the ordinary affine transformations used in this lesson, points normally use:

    w = 1

There is another useful case.

A **direction vector** can use:

    w = 0

Why is this interesting?

Consider the translation matrix:

$$
\begin{bmatrix}
1 & 0 & t_x \\
0 & 1 & t_y \\
0 & 0 & 1
\end{bmatrix}
$$

For a point:

$$
\begin{bmatrix}
x \\
y \\
1
\end{bmatrix}
$$

translation affects it:

$$
\begin{bmatrix}
x + t_x \\
y + t_y \\
1
\end{bmatrix}
$$

But for a direction:

$$
\begin{bmatrix}
x \\
y \\
0
\end{bmatrix}
$$

we get:

$$
\begin{bmatrix}
x \\
y \\
0
\end{bmatrix}
$$

because:

    tx × 0 = 0
    ty × 0 = 0

This makes sense:

> **Positions can be translated. Directions should not change simply because an object changes position.**

Later, when we introduce projection matrices, `w` becomes even more important.

After projection, `w` is not necessarily equal to `1`, and OpenGL eventually performs the **perspective divide**:

    x_ndc = x_clip / w_clip
    y_ndc = y_clip / w_clip
    z_ndc = z_clip / w_clip

We will cover that process properly in a future lesson.

---

# 7. The Model Matrix

A **model matrix** is the transformation that takes an object's vertices from **model space** into **world space**.

Mathematically:

$$
\mathbf{p}_{world} = M\mathbf{p}_{model}
$$

A model matrix commonly contains an object's:

- position
- rotation
- scale

Suppose:

    T = translation matrix
    R = rotation matrix
    S = scaling matrix

Using the column-vector convention used by GLSL and GLM, a common model matrix is:

$$
M = TRS
$$

Then:

$$
\mathbf{p}_{world}
=
TRS\mathbf{p}_{model}
$$

Because matrix multiplication applies from **right to left**, the transformations occur in this order:

    1. Scale
    2. Rotate
    3. Translate

Conceptually:

    model-space point
        ↓
       S
        ↓
    scaled point
        ↓
       R
        ↓
    rotated point
        ↓
       T
        ↓
    world-space point

---

## 7.1 Transformation Order

Matrix multiplication is **not commutative**.

In general:

$$
AB \neq BA
$$

This means:

    T × R

is generally not the same transformation as:

    R × T

Consider an object away from the origin.

If we:

    rotate
        ↓
    translate

we may get one result.

If we:

    translate
        ↓
    rotate

the translation itself may be rotated around the origin, producing a completely different result.

Therefore, transformation order is extremely important.

For the conventions used in this lesson:

```cpp
glm::mat3 model = translation * rotation * scale;
```

means that a vertex is transformed in this order:

    scale
        ↓
    rotation
        ↓
    translation

because the matrix closest to the vector is applied first.

> **`T * R * S` is a common model-matrix arrangement, but it is not a universal rule for every possible transformation. The correct order depends on the behavior you want and the vector/matrix conventions being used.**

---

## One Model Matrix Per Object or Instance

A model matrix describes a transformation.

Different objects can therefore have different model matrices:

    Object A
    position = (-4, 2)
    rotation = 20°
    scale    = (1, 1)

    Object B
    position = (3, -5)
    rotation = 90°
    scale    = (2, 2)

Their model matrices will differ.

However, it is not correct to say that every object must always have a mathematically unique model matrix.

Two objects can share the same transformation.

For example:

    Object A model matrix = M
    Object B model matrix = M

They could still contain completely different geometry.

The model matrix only describes how model-space positions are transformed into world space.

---

# 8. Shader Uniforms

We now know how to describe an object's transformation using a matrix.

The next question is:

> How do we give that matrix to the vertex shader?

This is where **uniforms** come in.

A uniform is a variable declared inside a GLSL shader whose value can be supplied by our application.

For example:

```glsl
uniform mat3 u_transform;
```

The important characteristic of a normal uniform is that its value is **constant across shader invocations for a particular draw operation unless the application changes it**.

For example, if we render one triangle using:

    u_transform = top_left_matrix

then every vertex processed by that draw call sees the same `u_transform`.

We can then change the uniform:

    u_transform = bottom_right_matrix

and issue another draw call.

The same geometry will now be transformed differently.

---

## 8.1 Adding a Uniform to the Vertex Shader

Our vertex shader can look like this:

```glsl
#version 460 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;

uniform mat3 u_transform;

out vec4 our_color;

void main()
{
    vec3 transformed_position =
        u_transform * vec3(a_pos, 1.0);

    gl_Position = vec4(
        transformed_position.xy,
        0.0,
        1.0
    );

    our_color = a_color;
}
```

The new declaration is:

```glsl
uniform mat3 u_transform;
```

`mat3` means:

    3 × 3 floating-point matrix

This matches the homogeneous 2D transformation matrices introduced earlier.

Our original vertex position is:

```glsl
a_pos
```

which is a `vec2`.

To use it with a `mat3`, we convert it into a homogeneous point:

```glsl
vec3(a_pos, 1.0)
```

If:

```glsl
a_pos = vec2(x, y)
```

then:

```glsl
vec3(a_pos, 1.0)
```

becomes:

    (x, y, 1)

We then perform:

```glsl
u_transform * vec3(a_pos, 1.0)
```

which applies our transformation matrix.

Finally:

```glsl
gl_Position = vec4(
    transformed_position.xy,
    0.0,
    1.0
);
```

converts the resulting 2D position into the four-component clip-space position required by OpenGL.

---

## 8.2 Uniform Values and Draw Calls

Suppose we have one triangle stored in our VBO.

We can render it once with:

    Transformation A

and then render the same geometry again with:

    Transformation B

Conceptually:

    Set uniform = A
        ↓
    draw triangle
        ↓
    triangle appears at A

    Set uniform = B
        ↓
    draw triangle
        ↓
    triangle appears at B

No new vertex buffer is required.

The exact same geometry is reused.

This pattern becomes extremely important in rendering.

---

# 9. Using GLM

Instead of implementing our own vector and matrix types, we are going to use **GLM**.

[GLM](https://github.com/g-truc/glm) is a mathematics library designed around graphics programming and GLSL-style types.

It provides types such as:

```cpp
glm::vec2
glm::vec3
glm::vec4

glm::mat2
glm::mat3
glm::mat4
```

Later, GLM will also provide convenient transformation functions for 3D graphics.

For now, we are going to manually construct our `mat3` matrices so that their structure remains clear.

---

# 9.1 Creating a Translation Matrix

Our translation function is:

```cpp
glm::mat3 create_translation_2d(const glm::vec2& translation)
{
    glm::mat3 result(1.0f);

    result[2] = glm::vec3(
        translation.x,
        translation.y,
        1.0f
    );

    return result;
}
```

The resulting matrix is conceptually:

$$
\begin{bmatrix}
1 & 0 & t_x \\
0 & 1 & t_y \\
0 & 0 & 1
\end{bmatrix}
$$

There is an important GLM convention to understand.

When we write:

```cpp
result[2]
```

GLM's matrix indexing gives us the **third column**, not the third row.

Therefore:

```cpp
result[2] = glm::vec3(tx, ty, 1.0f);
```

places the translation in the third column.

This matches the column-vector convention we are using:

$$
M\mathbf{v}
$$

---

## Translation Is an Offset

The argument:

```cpp
translation
```

represents a translation amount.

For example:

```cpp
glm::vec2 translation{ 10.0f, -6.0f };
```

means:

    move +10 along X
    move -6 along Y

If the object's current world position is effectively:

    (4, 7)

then applying that translation would move it to:

    (14, 1)

depending on the rest of its transformation hierarchy.

A translation vector should therefore be understood fundamentally as a **displacement**.

If your engine stores an object's position and constructs its model matrix from that position relative to the world origin, the position value can be placed directly into the translation component.

That does not change what the matrix itself does: it translates points by the specified amount.

---

# 9.2 Creating a Rotation Matrix

Our rotation function is:

```cpp
glm::mat3 create_rotation_2d(float angle_in_radians)
{
    const float cosine = std::cos(angle_in_radians);
    const float sine   = std::sin(angle_in_radians);

    glm::mat3 result(1.0f);

    result[0] = glm::vec3(
         cosine,
         sine,
         0.0f
    );

    result[1] = glm::vec3(
        -sine,
         cosine,
         0.0f
    );

    return result;
}
```

This produces:

$$
\begin{bmatrix}
\cos\theta & -\sin\theta & 0 \\
\sin\theta &  \cos\theta & 0 \\
0          &  0          & 1
\end{bmatrix}
$$

The angle is expected in **radians**.

If we want a `90°` rotation:

```cpp
const float rotation_angle =
    glm::radians(90.0f);
```

Using `glm::radians()` makes the conversion explicit.

---

# 9.3 Creating a Scaling Matrix

Our scaling function is:

```cpp
glm::mat3 create_scale_2d(const glm::vec2& scale)
{
    glm::mat3 result(1.0f);

    result[0][0] = scale.x;
    result[1][1] = scale.y;

    return result;
}
```

This creates:

$$
\begin{bmatrix}
s_x & 0   & 0 \\
0   & s_y & 0 \\
0   & 0   & 1
\end{bmatrix}
$$

For example:

```cpp
const glm::vec2 non_uniform_scale{
    0.5f,
    1.5f
};
```

means:

    X scale = 0.5
    Y scale = 1.5

The object becomes narrower along X and taller along Y.

---

# 10. Creating the Transformations

Our final image contains four triangles.

Define the values we want to use:

```cpp
const float rotation_angle =
    glm::radians(90.0f);

const glm::vec2 non_uniform_scale{
    0.5f,
    1.5f
};

const glm::vec2 top_left_position{
    -0.5f,
     0.5f
};

const glm::vec2 bottom_left_position{
    -0.5f,
    -0.5f
};

const glm::vec2 top_right_position{
     0.5f,
     0.5f
};

const glm::vec2 bottom_right_position{
     0.5f,
    -0.5f
};
```

Now construct the transformation matrices:

```cpp
const glm::mat3 translate_top_left =
    create_translation_2d(top_left_position);

const glm::mat3 translate_bottom_left =
    create_translation_2d(bottom_left_position);

const glm::mat3 translate_top_right =
    create_translation_2d(top_right_position);

const glm::mat3 translate_bottom_right =
    create_translation_2d(bottom_right_position);

const glm::mat3 rotate_2d =
    create_rotation_2d(rotation_angle);

const glm::mat3 scale_2d =
    create_scale_2d(non_uniform_scale);
```

We can now create four model matrices:

```cpp
const glm::mat3 top_left =
    translate_top_left;

const glm::mat3 bottom_left =
    translate_bottom_left *
    rotate_2d;

const glm::mat3 top_right =
    translate_top_right *
    scale_2d;

const glm::mat3 bottom_right =
    translate_bottom_right *
    rotate_2d *
    scale_2d;
```

Their behavior is:

| Matrix | Transformations |
|---|---|
| `top_left` | Translation |
| `bottom_left` | Rotation → Translation |
| `top_right` | Scale → Translation |
| `bottom_right` | Scale → Rotation → Translation |

Remember that the order above describes what happens to a column vector.

For example:

```cpp
translate_bottom_right *
rotate_2d *
scale_2d
```

is applied as:

    scale
        ↓
    rotate
        ↓
    translate

---

# 11. Sending a Matrix to the Shader

We now have:

```cpp
glm::mat3 top_left;
glm::mat3 bottom_left;
glm::mat3 top_right;
glm::mat3 bottom_right;
```

and our vertex shader contains:

```glsl
uniform mat3 u_transform;
```

We need to connect the two.

There are two main steps:

1. Find the uniform's location.
2. Upload the matrix.

---

# 11.1 `glGetUniformLocation`

We can retrieve the uniform location using:

```cpp
const GLint transform_location =
    glGetUniformLocation(
        shader_program,
        "u_transform"
    );
```

The first argument:

```cpp
shader_program
```

identifies the linked shader program.

The second:

```cpp
"u_transform"
```

must match the GLSL uniform name.

Our vertex shader contains:

```glsl
uniform mat3 u_transform;
```

so the names match.

The return type is:

```cpp
GLint
```

rather than an unsigned integer because the function can return:

```cpp
-1
```

A location of `-1` means that no active uniform with that name was found.

This can happen because:

- the name is incorrect
- the uniform does not exist
- the uniform was declared but is inactive and optimized away

For example:

```cpp
glGetUniformLocation(
    shader_program,
    "u_transformmm"
);
```

would return `-1` if no active uniform has that name.

---

## Cache Uniform Locations

Uniform locations normally only need to be queried after the shader program has been successfully linked.

There is usually no reason to call:

```cpp
glGetUniformLocation(...)
```

every frame.

A better approach is:

    Link shader program
        ↓
    query uniform location once
        ↓
    store location
        ↓
    reuse location while rendering

For example:

```cpp
const GLint transform_location =
    glGetUniformLocation(
        shader_program,
        "u_transform"
    );
```

can be performed during initialization.

---

# 11.2 `glUniformMatrix3fv`

To upload a `mat3`, we use:

```cpp
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(top_left)
);
```

Let's examine each argument.

---

## Location

```cpp
transform_location
```

is the location returned by:

```cpp
glGetUniformLocation(...)
```

---

## Count

```cpp
1
```

specifies how many matrices we are uploading.

We are uploading one `mat3`, so:

    count = 1

---

## Transpose

```cpp
GL_FALSE
```

tells OpenGL not to transpose the matrix while reading it.

GLM and the GLSL conventions we are using work correctly with:

```cpp
GL_FALSE
```

so that is what we should use here.

---

## Value

```cpp
glm::value_ptr(top_left)
```

returns a pointer to the matrix data in a form that OpenGL can read.

To use `glm::value_ptr()`, include:

```cpp
#include <glm/gtc/type_ptr.hpp>
```

Therefore:

```cpp
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(top_left)
);
```

means:

> Upload one `3 × 3` floating-point matrix to the uniform at `transform_location`.

---

## The Shader Program Must Be Active

Traditional `glUniform*()` functions modify a uniform belonging to the **currently active shader program**.

Therefore, before:

```cpp
glUniformMatrix3fv(...)
```

we need the correct program active:

```cpp
glUseProgram(shader_program);
```

For example:

```cpp
glUseProgram(shader_program);

glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(top_left)
);
```

This distinction is important.

`glGetUniformLocation()` takes the shader-program ID explicitly.

`glUniformMatrix3fv()` does not.

The latter operates on the currently active program.

---

## Different Uniform Functions

OpenGL provides different functions depending on the GLSL uniform type.

For example:

```cpp
glUniform1i(...)
```

can upload an integer.

```cpp
glUniform1f(...)
```

can upload a float.

```cpp
glUniform3fv(...)
```

can upload one or more `vec3` values.

```cpp
glUniformMatrix3fv(...)
```

can upload `mat3` values.

```cpp
glUniformMatrix4fv(...)
```

can upload `mat4` values.

Later, when we begin using 3D transformations, `mat4` and:

```cpp
glUniformMatrix4fv(...)
```

will become extremely common.

---

# 12. Rendering Multiple Transformed Objects

We now have everything we need.

Our VBO contains one triangle.

Our shader accepts one transformation matrix:

```glsl
uniform mat3 u_transform;
```

Our application has four matrices:

```cpp
top_left
bottom_left
top_right
bottom_right
```

We can therefore render the same geometry four times.

First, bind the shader and VAO:

```cpp
glUseProgram(shader_program);
glBindVertexArray(vao);
```

Render the top-left triangle:

```cpp
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(top_left)
);

glDrawArrays(
    GL_TRIANGLES,
    0,
    3
);
```

Render the bottom-left triangle:

```cpp
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(bottom_left)
);

glDrawArrays(
    GL_TRIANGLES,
    0,
    3
);
```

Render the top-right triangle:

```cpp
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(top_right)
);

glDrawArrays(
    GL_TRIANGLES,
    0,
    3
);
```

Render the bottom-right triangle:

```cpp
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(bottom_right)
);

glDrawArrays(
    GL_TRIANGLES,
    0,
    3
);
```

The important thing to notice is what **does not change**.

We do not:

- create another VBO
- copy the triangle four times
- modify the original vertex positions
- create four different vertex shaders

We simply change:

```cpp
u_transform
```

and issue another draw call.

Conceptually:

    One Triangle VBO
          │
          ├── model matrix A → draw
          │
          ├── model matrix B → draw
          │
          ├── model matrix C → draw
          │
          └── model matrix D → draw

This is one of the fundamental patterns used throughout real-time rendering.

---

## Complete Rendering Example

The relevant initialization code could look similar to:

```cpp
const float rotation_angle =
    glm::radians(90.0f);

const glm::vec2 non_uniform_scale{
    0.5f,
    1.5f
};

const glm::vec2 top_left_position{
    -0.5f,
     0.5f
};

const glm::vec2 bottom_left_position{
    -0.5f,
    -0.5f
};

const glm::vec2 top_right_position{
     0.5f,
     0.5f
};

const glm::vec2 bottom_right_position{
     0.5f,
    -0.5f
};

const glm::mat3 translate_top_left =
    create_translation_2d(top_left_position);

const glm::mat3 translate_bottom_left =
    create_translation_2d(bottom_left_position);

const glm::mat3 translate_top_right =
    create_translation_2d(top_right_position);

const glm::mat3 translate_bottom_right =
    create_translation_2d(bottom_right_position);

const glm::mat3 rotate_2d =
    create_rotation_2d(rotation_angle);

const glm::mat3 scale_2d =
    create_scale_2d(non_uniform_scale);

const glm::mat3 top_left =
    translate_top_left;

const glm::mat3 bottom_left =
    translate_bottom_left *
    rotate_2d;

const glm::mat3 top_right =
    translate_top_right *
    scale_2d;

const glm::mat3 bottom_right =
    translate_bottom_right *
    rotate_2d *
    scale_2d;

const GLint transform_location =
    glGetUniformLocation(
        shader_program,
        "u_transform"
    );
```

Then inside the rendering loop:

```cpp
glUseProgram(shader_program);
glBindVertexArray(vao);

// Top-left
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(top_left)
);

glDrawArrays(
    GL_TRIANGLES,
    0,
    3
);

// Bottom-left
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(bottom_left)
);

glDrawArrays(
    GL_TRIANGLES,
    0,
    3
);

// Top-right
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(top_right)
);

glDrawArrays(
    GL_TRIANGLES,
    0,
    3
);

// Bottom-right
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(bottom_right)
);

glDrawArrays(
    GL_TRIANGLES,
    0,
    3
);
```

The flow is:

    Use shader
        ↓
    Bind geometry
        ↓
    Upload model matrix A
        ↓
    Draw
        ↓
    Upload model matrix B
        ↓
    Draw
        ↓
    Upload model matrix C
        ↓
    Draw
        ↓
    Upload model matrix D
        ↓
    Draw

The vertex data is reused for every draw.

---

# 13. Common Transformation Mistakes

Transformations introduce several concepts that are easy to misunderstand at first.

---

## Mistake 1: Thinking Translation Stores an Absolute Destination

A translation matrix represents a **displacement**:

    (tx, ty)

means:

    x' = x + tx
    y' = y + ty

If your object's local origin starts at `(0, 0)`, then using:

    translation = (5, 2)

places that origin at:

    (5, 2)

This can make the translation appear to represent an absolute position.

Mathematically, however, the matrix is applying an offset.

---

## Mistake 2: Thinking Transformations Require New Geometry

Moving an object does not mean that we need another mesh.

The same VBO can be used with many different model matrices.

For example:

    Mesh
     │
     ├── M1
     ├── M2
     ├── M3
     └── M4

This is one of the primary reasons we separate model-space geometry from world-space placement.

---

## Mistake 3: Thinking the Model Origin Must Be the Center

Model space is relative to the object's chosen local coordinate system.

The origin:

    (0, 0)

does not have to be the geometric center.

If a mesh is authored with its origin at one corner, scaling and rotation will occur relative to that origin unless additional transformations are used.

The chosen pivot matters.

---

## Mistake 4: Applying Matrices in the Wrong Order

These are generally different:

```cpp
translation * rotation
```

and:

```cpp
rotation * translation
```

Matrix multiplication is not commutative.

With column vectors:

```cpp
M = T * R * S;
```

means:

    S happens first
    R happens second
    T happens last

Always think about the transformation order carefully.

---

## Mistake 5: Confusing Matrix Storage with Mathematical Layout

GLM uses column-major conventions by default, and:

```cpp
matrix[index]
```

accesses a **column**.

For example:

```cpp
result[2]
```

accesses the third column of a `glm::mat3`.

That is why our translation values are stored using:

```cpp
result[2] =
    glm::vec3(tx, ty, 1.0f);
```

---

## Mistake 6: Forgetting the Homogeneous Coordinate

Our shader input is:

```glsl
vec2 a_pos;
```

but our transformation matrix is:

```glsl
mat3 u_transform;
```

We therefore cannot directly multiply:

```glsl
u_transform * a_pos
```

Instead, convert the position into a homogeneous point:

```glsl
u_transform * vec3(a_pos, 1.0)
```

The `1.0` allows translation to affect the point.

---

## Mistake 7: Querying the Uniform Location Every Frame

This technically works:

```cpp
while (...)
{
    GLint location =
        glGetUniformLocation(
            shader_program,
            "u_transform"
        );

    // ...
}
```

but the location does not need to be repeatedly queried while the same linked shader program remains unchanged.

Prefer:

```cpp
const GLint transform_location =
    glGetUniformLocation(
        shader_program,
        "u_transform"
    );
```

once after linking the program.

Then reuse:

```cpp
transform_location
```

while rendering.

---

## Mistake 8: Forgetting `glUseProgram()`

This:

```cpp
glUniformMatrix3fv(...)
```

operates on the currently active shader program.

Therefore:

```cpp
glUseProgram(shader_program);
```

must have selected the correct program before calling the traditional `glUniform*()` functions.

---

## Mistake 9: Assuming `-1` Always Means a Typo

This function:

```cpp
glGetUniformLocation(...)
```

can return:

```cpp
-1
```

if the uniform cannot be found as an **active uniform**.

A spelling mistake is one possible cause.

Another is that the shader compiler/linker determined that the uniform does not affect the final output and optimized it away.

For example:

```glsl
uniform float unused_value;
```

may not receive a usable location if it is never used.

---

# 14. Conclusion

This lesson introduced one of the most important concepts in graphics programming: **transformations**.

We started with geometry defined in model space:

    model-space geometry
        ↓
    model transformation
        ↓
    world-space placement

and learned that we do not need to rewrite an object's vertex data whenever we want to move, rotate, or scale it.

Instead, we can describe the object's transformation mathematically.

---

## The Three Basic Transformations

We introduced:

| Transformation | Purpose |
|---|---|
| **Translation** | Moves an object |
| **Rotation** | Changes its orientation |
| **Scaling** | Changes its size |

These transformations can all be represented using matrices.

---

## Why Homogeneous Coordinates?

Ordinary `2 × 2` matrices can represent:

- rotation
- scaling

but they cannot represent translation.

By extending a 2D point:

$$
\begin{bmatrix}
x \\
y
\end{bmatrix}
$$

into homogeneous coordinates:

$$
\begin{bmatrix}
x \\
y \\
1
\end{bmatrix}
$$

we can use `3 × 3` matrices for all three transformations.

This allows us to create one combined transformation.

---

## The Model Matrix

The **model matrix** converts model-space positions into world-space positions:

$$
\mathbf{p}_{world}
=
M\mathbf{p}_{model}
$$

A common model matrix using our conventions is:

$$
M = TRS
$$

which applies:

    Scale
        ↓
    Rotation
        ↓
    Translation

Remember:

> **Matrix multiplication order matters.**

---

## Uniforms

We also introduced shader **uniforms**.

A uniform allows the application to provide a value to a shader.

Our vertex shader contains:

```glsl
uniform mat3 u_transform;
```

and applies it using:

```glsl
vec3 transformed_position =
    u_transform * vec3(a_pos, 1.0);
```

Our C++ program finds the uniform:

```cpp
const GLint transform_location =
    glGetUniformLocation(
        shader_program,
        "u_transform"
    );
```

and uploads a matrix:

```cpp
glUniformMatrix3fv(
    transform_location,
    1,
    GL_FALSE,
    glm::value_ptr(model_matrix)
);
```

---

## The Complete Idea

The entire process can now be represented as:

    Vertex Data
        ↓
    VBO
        ↓
    Vertex Shader
        ↓
    Model Matrix Uniform
        ↓
    Transform Vertex
        ↓
    gl_Position
        ↓
    Remaining Graphics Pipeline

Most importantly, the same geometry can be reused:

    One Mesh
       │
       ├── Model Matrix A → Object A
       ├── Model Matrix B → Object B
       ├── Model Matrix C → Object C
       └── Model Matrix D → Object D

That principle scales far beyond our four triangles.

A real scene may contain hundreds or thousands of objects, each using its own transformation while sharing meshes, shaders, textures, and other resources.

The main ideas to remember from this lesson are:

> **Model space describes the geometry relative to its local coordinate system.**

> **The model matrix transforms that geometry into world space.**

> **Translation, rotation, and scaling can be combined using matrix multiplication.**

> **Homogeneous coordinates allow translation to participate in the same matrix framework as rotation and scaling.**

> **Uniforms allow our application to provide transformation matrices and other values to shaders.**

With transformations in place, our objects no longer need to remain static. We now have the mathematical and OpenGL foundations required to begin building an actual scene.