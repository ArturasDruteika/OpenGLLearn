# Lesson 3 — Element Buffer Objects

In the previous lesson, we learned how to store vertex data in a **Vertex Buffer Object (VBO)** and describe that data using a **Vertex Array Object (VAO)**.

In this lesson, we are going to introduce another important OpenGL concept: the **Element Buffer Object (EBO)**.

An EBO is also commonly referred to as an **Index Buffer Object**.

Its purpose is simple:

> **An EBO allows multiple primitives to reuse the same vertex data by referencing vertices through indices.**

This technique is called **indexed rendering**.

By the end of this lesson, you should understand:

1. What an EBO is.
2. Why indexed rendering is useful.
3. What vertex indices are.
4. How an EBO relates to a VBO and VAO.
5. How to upload index data to OpenGL.
6. How to render indexed geometry using `glDrawElements()`.

---

# Contents

1. [What Is an Element Buffer Object?](#1-what-is-an-element-buffer-object)
2. [The Problem: Duplicate Vertex Data](#2-the-problem-duplicate-vertex-data)
3. [Reusing Vertices with Indices](#3-reusing-vertices-with-indices)
4. [Memory Comparison](#4-memory-comparison)
5. [Vertex Sharing in 3D](#5-vertex-sharing-in-3d)
6. [Implementing an EBO in OpenGL](#6-implementing-an-ebo-in-opengl)
   - [6.1 Define the Vertex Data](#61-define-the-vertex-data)
   - [6.2 Define the Index Data](#62-define-the-index-data)
   - [6.3 Create the VAO, VBO, and EBO](#63-create-the-vao-vbo-and-ebo)
   - [6.4 Upload the Vertex Data](#64-upload-the-vertex-data)
   - [6.5 Upload the Index Data](#65-upload-the-index-data)
   - [6.6 Configure the Vertex Attributes](#66-configure-the-vertex-attributes)
7. [The Relationship Between an EBO and a VAO](#7-the-relationship-between-an-ebo-and-a-vao)
8. [Drawing with glDrawElements](#8-drawing-with-gldrawelements)
   - [8.1 Primitive Type](#81-primitive-type)
   - [8.2 Index Count](#82-index-count)
   - [8.3 Index Type](#83-index-type)
   - [8.4 Index Offset](#84-index-offset)
9. [glDrawArrays vs. glDrawElements](#9-gldrawarrays-vs-gldrawelements)
10. [Complete EBO Setup](#10-complete-ebo-setup)
11. [Common EBO Mistakes](#11-common-ebo-mistakes)
12. [Deleting the OpenGL Objects](#12-deleting-the-opengl-objects)
13. [Conclusion](#13-conclusion)

---

# 1. What Is an Element Buffer Object?

An **Element Buffer Object (EBO)** is a buffer object that stores **indices** used during indexed rendering.

Technically, OpenGL does not create a special type of buffer object called an "EBO".

Both VBOs and EBOs are created using:

```cpp
glGenBuffers(...);
```

They are both ordinary OpenGL **buffer objects**.

The terms **VBO** and **EBO** describe how we use those buffers.

A buffer used for vertex data is commonly bound to:

```cpp
GL_ARRAY_BUFFER
```

while a buffer used for vertex indices is bound to:

```cpp
GL_ELEMENT_ARRAY_BUFFER
```

Conceptually:

    VBO
    └── stores vertex attributes

    EBO
    └── stores indices that reference those vertices

The purpose of the EBO becomes clearer when multiple triangles share vertices.

Suppose our vertex format contains:

    2 floats → position
    4 floats → color

That gives us:

    6 floats per vertex

A single triangle requires three vertices:

    3 vertices × 6 floats = 18 floats

For example:

```cpp
float triangle_vertices[] = {
    // positions    // colors
     0.0f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // bottom right
};
```

For a completely independent triangle, storing three separate vertices is perfectly reasonable.

The problem appears when multiple triangles need to use the **same vertices**.

---

# 2. The Problem: Duplicate Vertex Data

Imagine that we want to render a square.

A square can be constructed from two triangles:

    top left ---------------- top right
       |                    /   |
       |                  /     |
       |                /       |
       |              /         |
       |            /           |
       |          /             |
       |        /               |
       |      /                 |
       |    /                   |
       |  /                     |
    bottom left ------------ bottom right

For this lesson, we are rendering using `GL_TRIANGLES`, so every triangle requires three vertices.

Without indexed rendering, we could describe the square using six vertex entries:

```cpp
float square_vertices[] = {
    // positions    // colors

    // Triangle 1
    -0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top left
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom right

    // Triangle 2
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // bottom right
     0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // top right
    -0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // top left
};
```

Take a closer look at the data.

The **top-left** vertex appears twice.

The **bottom-right** vertex also appears twice.

Both triangles use those vertices, but without indices we have stored complete copies of their data.

![Same Vertices](assets/square__same_vertices.png)

For a square, duplicating two vertices is insignificant.

However, larger meshes can contain thousands or millions of triangles, and neighboring triangles frequently share vertices.

If complete vertex records are duplicated unnecessarily, we may increase:

- vertex-buffer size
- GPU memory usage
- vertex-data bandwidth
- the amount of vertex processing required

This is where **indexed rendering** becomes useful.

---

# 3. Reusing Vertices with Indices

Instead of storing six complete vertex entries, we can store the four unique vertices of the square:

```cpp
float square_vertices[] = {
    // positions    // colors
    -0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 0 - top left
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 1 - bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 2 - bottom right
     0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // 3 - top right
};
```

Each complete vertex now has an index:

    0 → top left
    1 → bottom left
    2 → bottom right
    3 → top right

We then create a separate array that describes which vertices should be used to construct our triangles:

```cpp
unsigned int indices[] = {
    0, 1, 2, // triangle 1
    2, 3, 0  // triangle 2
};
```

The first triangle uses:

    0, 1, 2

which means:

    top left
        ↓
    bottom left
        ↓
    bottom right

The second triangle uses:

    2, 3, 0

which means:

    bottom right
        ↓
    top right
        ↓
    top left

Notice that indices `0` and `2` appear more than once.

That does **not** mean the vertex data itself appears more than once.

The VBO still contains only four complete vertices:

    VBO:

    vertex 0 → top left
    vertex 1 → bottom left
    vertex 2 → bottom right
    vertex 3 → top right

The EBO tells OpenGL how to reuse them:

    EBO:

    0, 1, 2,
    2, 3, 0

This is the fundamental idea behind indexed rendering:

> **Store reusable vertex data once and reference it multiple times using indices.**

---

# 4. Memory Comparison

Let's compare the amount of data required for our square.

Our vertex format contains:

    2 floats → position
    4 floats → color

Therefore:

    6 floats per vertex

For this example, we will assume:

    float        = 4 bytes
    unsigned int = 4 bytes

These are the typical sizes used with the corresponding OpenGL data types in this lesson.

## Without Indexed Rendering

The non-indexed square contains six complete vertex entries:

    6 vertices × 6 floats = 36 floats

At 4 bytes per float:

    36 × 4 = 144 bytes

Therefore:

    Vertex data = 144 bytes

There is no index buffer.

So the total is:

    144 bytes

## With Indexed Rendering

The indexed version contains four complete vertices:

    4 vertices × 6 floats = 24 floats

At 4 bytes per float:

    24 × 4 = 96 bytes

The EBO contains six `unsigned int` indices:

    6 indices × 4 bytes = 24 bytes

Therefore:

    Vertex data = 96 bytes
    Index data  = 24 bytes

    Total = 120 bytes

Our comparison is:

| Representation | Vertex Data | Index Data | Total |
|---|---:|---:|---:|
| Non-indexed | 144 bytes | 0 bytes | 144 bytes |
| Indexed | 96 bytes | 24 bytes | 120 bytes |
| **Difference** | | | **24 bytes saved** |

For a square, saving 24 bytes is obviously not important.

The purpose of this example is to demonstrate the concept.

On larger meshes, vertex records may contain considerably more information than just a position and color.

For example:

    position
    normal
    texture coordinates
    tangent
    bitangent
    bone IDs
    bone weights
    ...

The larger each complete vertex becomes, the more valuable vertex reuse can become.

> **Important:** You cannot calculate the memory savings of indexed rendering from the number of triangles alone.

Two meshes containing the same number of triangles may contain very different numbers of unique vertices.

The amount of memory saved depends on:

- mesh topology
- the number of reusable vertices
- the size of each vertex
- the chosen index type

---

# 5. Vertex Sharing in 3D

There is an important detail about vertex reuse that becomes especially relevant in 3D graphics.

Suppose a vertex contains:

    x, y, z       → position
    r, g, b, a    → color

That gives us:

    3 position floats
    +
    4 color floats
    =
    7 floats per vertex

Later, our vertices may contain additional attributes such as:

- texture coordinates
- normals
- tangents
- bitangents
- bone IDs
- bone weights

At that point, two vertices having the same **position** does not necessarily mean that they are the same complete vertex.

For example, imagine two faces of a cube meeting at one corner.

The position might be identical:

    Position:
    (0.5, 0.5, 0.5)

but each face may require a different normal:

    Face A normal:
    (1.0, 0.0, 0.0)

    Face B normal:
    (0.0, 1.0, 0.0)

The positions are identical, but the complete vertex records are not.

Similarly, the same position may require different texture coordinates on different faces.

Therefore:

> **An index identifies a complete vertex record, not only a position.**

If any attribute that belongs to the vertex must be different, we generally need a separate vertex entry.

This becomes especially important when working with:

- hard edges
- texture seams
- cube faces
- imported 3D models
- normal mapping

For now, remember:

> **Vertices can only be shared when the complete set of vertex attributes we need is compatible with that sharing.**

---

# 6. Implementing an EBO in OpenGL

We now understand the purpose of indexed rendering.

Let's implement it.

We are going to use three OpenGL objects:

    VAO → stores vertex-input state
    VBO → stores vertex data
    EBO → stores vertex indices

---

## 6.1 Define the Vertex Data

First, define the four unique vertices of the square:

```cpp
float square_vertices[] = {
    // positions    // colors
    -0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 0 - top left
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 1 - bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 2 - bottom right
     0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // 3 - top right
};
```

Our VBO will eventually contain this data.

Conceptually:

    VBO
    │
    ├── vertex 0
    ├── vertex 1
    ├── vertex 2
    └── vertex 3

---

## 6.2 Define the Index Data

Next, define the indices:

```cpp
unsigned int indices[] = {
    0, 1, 2, // triangle 1
    2, 3, 0  // triangle 2
};
```

Our EBO will contain this data.

Conceptually:

    EBO
    │
    └── 0, 1, 2, 2, 3, 0

The two arrays therefore have different responsibilities:

    square_vertices
        → contains the actual vertex attributes

    indices
        → determines which vertices are used to construct the triangles

---

## 6.3 Create the VAO, VBO, and EBO

First, create variables that will store the OpenGL object names:

```cpp
unsigned int vao;
unsigned int vbo;
unsigned int ebo;
```

Generate the objects:

```cpp
glGenVertexArrays(1, &vao);
glGenBuffers(1, &vbo);
glGenBuffers(1, &ebo);
```

Notice that both the VBO and EBO are generated using:

```cpp
glGenBuffers(...);
```

This is because both are **buffer objects**.

`glGenBuffers()` does not create a "vertex buffer" or an "element buffer" specifically.

It generates a buffer object name.

How we use that buffer depends on which target we bind it to and which OpenGL operations use it.

For our VBO:

```cpp
glBindBuffer(GL_ARRAY_BUFFER, vbo);
```

For our EBO:

```cpp
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
```

The variables:

```cpp
vbo
ebo
```

do not contain the vertex or index data themselves.

They contain integer object names that OpenGL uses to identify the corresponding buffer objects.

---

## 6.4 Upload the Vertex Data

First, bind the VAO:

```cpp
glBindVertexArray(vao);
```

Then bind the VBO to `GL_ARRAY_BUFFER`:

```cpp
glBindBuffer(GL_ARRAY_BUFFER, vbo);
```

Now upload the vertex data:

```cpp
glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(square_vertices),
    square_vertices,
    GL_STATIC_DRAW
);
```

Let's review the arguments:

    GL_ARRAY_BUFFER
        → operate on the buffer currently bound
          to the GL_ARRAY_BUFFER target

    sizeof(square_vertices)
        → allocate enough storage for the entire
          square_vertices array

    square_vertices
        → copy data from this array into the buffer

    GL_STATIC_DRAW
        → usage hint indicating that the data is expected
          to change rarely and be used for drawing

It is worth emphasizing that `GL_STATIC_DRAW` is a **usage hint** given to the OpenGL implementation.

It does not make the buffer permanently immutable.

---

## 6.5 Upload the Index Data

Now bind the EBO to `GL_ELEMENT_ARRAY_BUFFER`:

```cpp
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
```

Upload the index data:

```cpp
glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    sizeof(indices),
    indices,
    GL_STATIC_DRAW
);
```

The process is nearly identical to uploading the VBO.

The major difference is the buffer target:

    VBO → GL_ARRAY_BUFFER
    EBO → GL_ELEMENT_ARRAY_BUFFER

After uploading both buffers, we conceptually have:

    VBO
    │
    ├── vertex 0
    ├── vertex 1
    ├── vertex 2
    └── vertex 3

    EBO
    │
    └── 0, 1, 2, 2, 3, 0

The EBO does not contain copies of the vertices.

It only contains indices that reference them.

---

## 6.6 Configure the Vertex Attributes

Our vertex layout contains:

    2 floats → position
    4 floats → color

Therefore, one complete vertex contains:

    6 floats

The stride between consecutive vertices is:

    6 × sizeof(float)

Our position attribute contains two components and starts at offset zero:

```cpp
glVertexAttribPointer(
    0,
    2,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    (void*)0
);

glEnableVertexAttribArray(0);
```

Our color attribute contains four components.

Because the position occupies the first two floats, the color starts at:

    2 × sizeof(float)

Therefore:

```cpp
glVertexAttribPointer(
    1,
    4,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    (void*)(2 * sizeof(float))
);

glEnableVertexAttribArray(1);
```

Our complete configuration is now:

```cpp
glBindVertexArray(vao);

// Upload vertex data
glBindBuffer(GL_ARRAY_BUFFER, vbo);

glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(square_vertices),
    square_vertices,
    GL_STATIC_DRAW
);

// Upload index data
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    sizeof(indices),
    indices,
    GL_STATIC_DRAW
);

// Position attribute
glVertexAttribPointer(
    0,
    2,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    (void*)0
);

glEnableVertexAttribArray(0);

// Color attribute
glVertexAttribPointer(
    1,
    4,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    (void*)(2 * sizeof(float))
);

glEnableVertexAttribArray(1);
```

---

# 7. The Relationship Between an EBO and a VAO

There is an important difference between how VBO and EBO bindings interact with a VAO.

The binding of:

```cpp
GL_ELEMENT_ARRAY_BUFFER
```

is part of the **VAO's state**.

Consider:

```cpp
glBindVertexArray(vao);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
```

Because `vao` is currently bound, the element-buffer binding becomes associated with that VAO.

Conceptually:

    VAO
    │
    ├── Vertex Attribute 0
    │   ├── format
    │   ├── stride
    │   ├── offset
    │   └── vertex-buffer association
    │
    ├── Vertex Attribute 1
    │   ├── format
    │   ├── stride
    │   ├── offset
    │   └── vertex-buffer association
    │
    └── Element Buffer Binding
        └── EBO

Later, when we bind the VAO again:

```cpp
glBindVertexArray(vao);
```

its element-buffer binding is restored.

This means that during rendering we normally do not need to bind the EBO separately every frame.

Binding the correct VAO is enough.

For example:

```cpp
glBindVertexArray(vao);
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
```

The VAO already knows which element buffer should be used.

## What About `GL_ARRAY_BUFFER`?

There is an important distinction here.

The current global `GL_ARRAY_BUFFER` binding is **not** simply stored in the VAO as an equivalent counterpart to the EBO binding.

Instead, when we call:

```cpp
glVertexAttribPointer(...);
```

OpenGL associates the buffer currently bound to `GL_ARRAY_BUFFER` with that particular vertex attribute configuration.

So, in the traditional vertex-attribute setup used in this lesson:

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
        ↓
    glVertexAttribPointer(...)
        ↓
    the attribute configuration records
    which buffer supplies its data

The EBO works differently:

    glBindVertexArray(vao);
        ↓
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        ↓
    the VAO stores that element-buffer binding

This is an important OpenGL state-management detail.

A useful rule for now is:

> **Bind the VAO before binding the EBO that should be associated with it.**

---

# 8. Drawing with `glDrawElements`

Before introducing an EBO, we rendered using `glDrawArrays()`.

For example:

```cpp
glDrawArrays(GL_TRIANGLES, 0, 3);
```

`glDrawArrays()` processes vertices sequentially.

For six vertices:

    vertex 0
    vertex 1
    vertex 2
        ↓
    triangle 1

    vertex 3
    vertex 4
    vertex 5
        ↓
    triangle 2

With indexed rendering, we instead want OpenGL to follow the indices stored in our EBO.

Therefore, we use:

```cpp
glDrawElements(
    GL_TRIANGLES,
    6,
    GL_UNSIGNED_INT,
    nullptr
);
```

The function has four important arguments:

    glDrawElements(
        primitive type,
        index count,
        index type,
        index offset
    );

Let's examine each one.

---

## 8.1 Primitive Type

The first argument is:

```cpp
GL_TRIANGLES
```

This tells OpenGL how the indexed vertices should be assembled into primitives.

With `GL_TRIANGLES`, every group of three indices forms an independent triangle.

Our EBO contains:

    0, 1, 2, 2, 3, 0

OpenGL interprets this as:

    0, 1, 2
    └───────┘
    triangle 1

    2, 3, 0
    └───────┘
    triangle 2

The EBO itself does not say "these are triangles."

The primitive mode passed to `glDrawElements()` determines how OpenGL interprets the index sequence.

---

## 8.2 Index Count

The second argument is:

```cpp
6
```

This tells OpenGL how many **indices** to process.

Our index array contains:

```cpp
unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};
```

There are six indices.

Therefore:

```cpp
glDrawElements(
    GL_TRIANGLES,
    6,
    GL_UNSIGNED_INT,
    nullptr
);
```

uses all six.

A common mistake is to pass the number of unique vertices instead.

For our square:

    Unique vertices = 4
    Indices         = 6
    Triangles       = 2

The `count` parameter must be:

    6

because it represents the number of **indices to read**, not the number of unique vertices.

---

## 8.3 Index Type

The third argument is:

```cpp
GL_UNSIGNED_INT
```

This tells OpenGL the data type stored inside the EBO.

Our C++ array is:

```cpp
unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};
```

Therefore, we use:

```cpp
GL_UNSIGNED_INT
```

`glDrawElements()` supports these index types:

    GL_UNSIGNED_BYTE
    GL_UNSIGNED_SHORT
    GL_UNSIGNED_INT

Their typical sizes are:

| OpenGL Type | Size | Maximum Index Value |
|---|---:|---:|
| `GL_UNSIGNED_BYTE` | 1 byte | 255 |
| `GL_UNSIGNED_SHORT` | 2 bytes | 65,535 |
| `GL_UNSIGNED_INT` | 4 bytes | 4,294,967,295 |

A smaller index type can reduce the size of the EBO.

For example, if a mesh contains no more than 65,536 addressable vertices for a draw, `GL_UNSIGNED_SHORT` may be sufficient.

For our beginner examples, `GL_UNSIGNED_INT` is convenient and easy to understand.

The important requirement is:

> **The type passed to `glDrawElements()` must match the type of the indices stored in the EBO.**

---

## 8.4 Index Offset

The final argument is:

```cpp
nullptr
```

When an element buffer is bound, this argument is interpreted as a **byte offset into the EBO**.

Passing:

```cpp
nullptr
```

represents an offset of zero.

Therefore:

```cpp
glDrawElements(
    GL_TRIANGLES,
    6,
    GL_UNSIGNED_INT,
    nullptr
);
```

means:

> Read 6 unsigned integer indices starting from byte offset 0 of the currently bound element buffer and use them to construct triangles.

In other words:

    EBO:

    offset 0
       ↓
    [0][1][2][2][3][0]

OpenGL begins reading from the first index.

Later, if one EBO contained indices for several groups of geometry, a non-zero offset could be used to begin reading from another location.

For now:

```cpp
nullptr
```

simply means:

> **Start at the beginning of the EBO.**

---

# 9. `glDrawArrays` vs. `glDrawElements`

We now have two different approaches to drawing geometry.

## `glDrawArrays()`

`glDrawArrays()` processes vertices sequentially.

For example:

```cpp
glDrawArrays(GL_TRIANGLES, 0, 6);
```

conceptually processes:

    0 → 1 → 2 → 3 → 4 → 5

With `GL_TRIANGLES`, OpenGL forms:

    0, 1, 2 → triangle 1
    3, 4, 5 → triangle 2

If the two triangles need identical complete vertices, those vertex entries still need to appear more than once in the vertex data.

---

## `glDrawElements()`

`glDrawElements()` follows indices stored in an element buffer.

For our square:

    0 → 1 → 2 → 2 → 3 → 0

The VBO contains only:

    vertex 0
    vertex 1
    vertex 2
    vertex 3

while the EBO contains:

    0, 1, 2,
    2, 3, 0

This allows vertices `0` and `2` to be referenced multiple times.

The basic difference is:

| Function | How Vertices Are Selected |
|---|---|
| `glDrawArrays()` | Sequentially |
| `glDrawElements()` | Through indices |

Indexed rendering has two major potential advantages.

### Reduced Vertex-Data Duplication

Shared complete vertex records can be stored once instead of being duplicated for every triangle that uses them.

### Reduced Repeated Vertex Processing

GPUs commonly contain a **post-transform vertex cache**.

When nearby indexed triangles reference the same vertex, the GPU may be able to reuse a previously calculated vertex-shader result rather than running the vertex shader again for that vertex.

Conceptually:

    index 2
        ↓
    process vertex 2
        ↓
    result may be cached
        ↓
    index 2 appears again
        ↓
    cached result may be reused

This is one reason why index ordering can matter for performance in larger meshes.

However, indexed rendering should not be understood as automatically faster in every possible situation.

Performance depends on many factors, including:

- mesh topology
- vertex reuse
- index ordering
- vertex size
- vertex shader complexity
- memory-access patterns
- GPU architecture

For now, the main idea is:

> **Indexed rendering is especially useful when multiple primitives can reuse the same complete vertices.**

---

# 10. Complete EBO Setup

Let's put everything together.

## Vertex and Index Data

```cpp
float square_vertices[] = {
    // positions    // colors
    -0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 0 - top left
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 1 - bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 2 - bottom right
     0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // 3 - top right
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};
```

## Generate the Objects

```cpp
unsigned int vao;
unsigned int vbo;
unsigned int ebo;

glGenVertexArrays(1, &vao);
glGenBuffers(1, &vbo);
glGenBuffers(1, &ebo);
```

## Bind the VAO

```cpp
glBindVertexArray(vao);
```

## Upload the Vertex Data

```cpp
glBindBuffer(GL_ARRAY_BUFFER, vbo);

glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(square_vertices),
    square_vertices,
    GL_STATIC_DRAW
);
```

## Upload the Index Data

```cpp
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    sizeof(indices),
    indices,
    GL_STATIC_DRAW
);
```

## Configure the Position Attribute

```cpp
glVertexAttribPointer(
    0,
    2,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    (void*)0
);

glEnableVertexAttribArray(0);
```

## Configure the Color Attribute

```cpp
glVertexAttribPointer(
    1,
    4,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    (void*)(2 * sizeof(float))
);

glEnableVertexAttribArray(1);
```

## Unbind the VAO

```cpp
glBindVertexArray(0);
```

The complete initialization code is therefore:

```cpp
float square_vertices[] = {
    // positions    // colors
    -0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 0 - top left
    -0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 1 - bottom left
     0.5f, -0.5f,   0.0f, 1.0f, 1.0f, 1.0f, // 2 - bottom right
     0.5f,  0.5f,   0.0f, 1.0f, 1.0f, 1.0f  // 3 - top right
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};

unsigned int vao;
unsigned int vbo;
unsigned int ebo;

glGenVertexArrays(1, &vao);
glGenBuffers(1, &vbo);
glGenBuffers(1, &ebo);

glBindVertexArray(vao);

// Upload vertex data
glBindBuffer(GL_ARRAY_BUFFER, vbo);

glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(square_vertices),
    square_vertices,
    GL_STATIC_DRAW
);

// Upload index data
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    sizeof(indices),
    indices,
    GL_STATIC_DRAW
);

// Position attribute
glVertexAttribPointer(
    0,
    2,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    (void*)0
);

glEnableVertexAttribArray(0);

// Color attribute
glVertexAttribPointer(
    1,
    4,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    (void*)(2 * sizeof(float))
);

glEnableVertexAttribArray(1);

// Unbind the VAO
glBindVertexArray(0);
```

Then, inside the rendering loop:

```cpp
glBindVertexArray(vao);

glDrawElements(
    GL_TRIANGLES,
    6,
    GL_UNSIGNED_INT,
    nullptr
);
```

Because the EBO is associated with the VAO, we do not need to bind the EBO again before every draw call.

Conceptually, the rendering process looks like this:

    Bind VAO
        ↓
    restore vertex-input configuration
        ↓
    restore associated EBO binding
        ↓
    call glDrawElements()
        ↓
    read indices from the EBO
        ↓
    indices select vertex records
        ↓
    vertex shader processes required vertices
        ↓
    OpenGL assembles the resulting vertices
    into triangles
        ↓
    triangles continue through the
    graphics pipeline

For our square:

    EBO:
    0, 1, 2, 2, 3, 0

        ↓

    Triangle 1:
    vertex 0
    vertex 1
    vertex 2

        ↓

    Triangle 2:
    vertex 2
    vertex 3
    vertex 0

---

# 11. Common EBO Mistakes

There are several mistakes that are easy to make when first working with indexed rendering.

## Mistake 1: Passing the Number of Vertices to `glDrawElements()`

Consider:

```cpp
glDrawElements(
    GL_TRIANGLES,
    6,
    GL_UNSIGNED_INT,
    nullptr
);
```

The second argument is the number of **indices**, not the number of unique vertices.

Our square has:

    4 unique vertices
    6 indices

Therefore:

    Correct count = 6

not:

    Incorrect count = 4

---

## Mistake 2: Using the Wrong Index Type

If the EBO contains:

```cpp
unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};
```

then the draw call should use:

```cpp
GL_UNSIGNED_INT
```

If the buffer instead contained an appropriate array of unsigned 16-bit indices, the corresponding OpenGL type would be:

```cpp
GL_UNSIGNED_SHORT
```

The type passed to `glDrawElements()` must describe the actual index representation stored in the EBO.

---

## Mistake 3: Forgetting That EBO Binding Is VAO State

Suppose we have:

```cpp
glBindVertexArray(vao);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
```

The EBO binding is now associated with that VAO.

If we later change or clear the element-buffer binding while the same VAO is bound, we also change that VAO's element-buffer state.

For example, this is usually **not** what we want during setup:

```cpp
glBindVertexArray(vao);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

// ...

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
```

because the final call changes the element-buffer binding stored in the currently bound VAO.

If you want to finish configuring the VAO, it is generally simpler to unbind the VAO:

```cpp
glBindVertexArray(0);
```

without first clearing its EBO binding.

---

## Mistake 4: Thinking an Index Refers Only to a Position

An index selects a **complete vertex**.

If your vertex contains:

    position
    color
    normal
    texture coordinates

then an index selects all of those attributes together.

Two vertices with the same position but different normals are not necessarily reusable as one vertex.

For example:

    Vertex A:
    position = (1, 1, 1)
    normal   = (1, 0, 0)

    Vertex B:
    position = (1, 1, 1)
    normal   = (0, 1, 0)

These must normally be represented as separate vertex entries because their complete attribute data differs.

---

## Mistake 5: Using an Invalid Index

Every index must refer to a valid vertex that can be fetched from the configured vertex buffers.

If we only have four vertices:

    valid indices:

    0
    1
    2
    3

then an index such as:

    7

would attempt to reference vertex data outside the intended range.

Your index data must therefore correspond correctly to the available vertex data.

---

# 12. Deleting the OpenGL Objects

When the objects are no longer needed, we should delete them.

Delete the EBO:

```cpp
glDeleteBuffers(1, &ebo);
```

Delete the VBO:

```cpp
glDeleteBuffers(1, &vbo);
```

Delete the VAO:

```cpp
glDeleteVertexArrays(1, &vao);
```

Together:

```cpp
glDeleteBuffers(1, &ebo);
glDeleteBuffers(1, &vbo);
glDeleteVertexArrays(1, &vao);
```

This releases the OpenGL objects when we no longer need them.

---

# 13. Conclusion

In this lesson, we introduced **indexed rendering** and the **Element Buffer Object**.

Let's summarize the most important concepts.

## What Is an EBO?

An EBO is a buffer object used to store **indices**.

The indices determine which vertices are used when OpenGL assembles primitives.

For example:

    VBO:

    vertex 0 → top left
    vertex 1 → bottom left
    vertex 2 → bottom right
    vertex 3 → top right

    EBO:

    0, 1, 2,
    2, 3, 0

The VBO contains the actual vertex attributes.

The EBO contains indices that reference those vertices.

---

## Why Use an EBO?

Without indexed rendering, two triangles might require:

    Triangle 1:

    A, B, C

    Triangle 2:

    C, D, A

Vertices `A` and `C` are used by both triangles.

Without indices, their complete data may need to be duplicated.

With indexed rendering, we can instead store:

    Vertices:

    0 → A
    1 → B
    2 → C
    3 → D

and reference them using:

    Indices:

    0, 1, 2
    2, 3, 0

The complete vertex records for `A` and `C` are stored once but referenced more than once.

---

## `glDrawArrays()` vs. `glDrawElements()`

`glDrawArrays()` processes vertices sequentially:

    0 → 1 → 2 → 3 → 4 → 5

`glDrawElements()` follows an index sequence:

    0 → 1 → 2 → 2 → 3 → 0

Therefore:

| Function | Vertex Selection |
|---|---|
| `glDrawArrays()` | Sequential |
| `glDrawElements()` | Index-based |

---

## VAO, VBO, and EBO

At this point, we have introduced three important OpenGL objects:

| Object | Purpose |
|---|---|
| **VBO** | Stores vertex data |
| **EBO** | Stores indices |
| **VAO** | Stores vertex-input configuration and the element-buffer binding |

A simplified conceptual representation is:

    VAO
    │
    ├── Vertex Attribute Configuration
    │       │
    │       └── VBO
    │           │
    │           └── Vertex Data
    │
    └── EBO
        │
        └── Indices

During indexed rendering:

    glBindVertexArray(vao)
        ↓
    glDrawElements(...)
        ↓
    read indices from EBO
        ↓
    select vertex records
        ↓
    process vertices
        ↓
    assemble primitives
        ↓
    continue through the
    graphics pipeline

The most important idea to remember from this lesson is:

> **A VBO stores vertex data. An EBO stores indices that select vertices. A VAO stores the vertex-input configuration and its associated element-buffer binding.**

With VAOs, VBOs, and EBOs together, we now have the basic OpenGL objects required to represent and render indexed geometry efficiently.