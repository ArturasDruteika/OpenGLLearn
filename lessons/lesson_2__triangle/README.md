# Lesson 2 — Rendering a Triangle

## Contents

- [Overview](#overview)
- [1. Triangle](#1-triangle)
  - [Triangle in Rendering](#triangle-in-rendering)
  - [Triangle Data](#triangle-data)
  - [Positions](#positions)
  - [Colors](#colors)
- [2. Rendering Steps](#2-rendering-steps)
- [3. Vertex Buffer Object — VBO](#3-vertex-buffer-object--vbo)
  - [Creating a VBO](#creating-a-vbo)
  - [Understanding Buffer Objects](#understanding-buffer-objects)
- [4. Vertex Array Object — VAO](#4-vertex-array-object--vao)
  - [Creating a VAO](#creating-a-vao)
  - [Describing Vertex Attributes](#describing-vertex-attributes)
- [5. Vertex and Fragment Shaders](#5-vertex-and-fragment-shaders)
  - [What Is a Shader?](#what-is-a-shader)
  - [Vertex Shader](#vertex-shader)
  - [Fragment Shader](#fragment-shader)
  - [Vertex Shader Breakdown](#vertex-shader-breakdown)
  - [Fragment Shader Breakdown](#fragment-shader-breakdown)
  - [Creating a Shader Program](#creating-a-shader-program)
  - [Shader Compilation](#shader-compilation)
- [6. Main Rendering Loop](#6-main-rendering-loop)
  - [Clearing the Framebuffer](#clearing-the-framebuffer)
  - [Issuing a Draw Call](#issuing-a-draw-call)
- [7. Framebuffer Size Callback](#7-framebuffer-size-callback)
- [Conclusion](#conclusion)

---

## Overview

Before starting, [LearnOpenGL's Hello Triangle](https://learnopengl.com/Getting-started/Hello-Triangle) chapter is an excellent additional resource covering many of the concepts introduced in this lesson.

This lesson combines several fundamental OpenGL concepts rather than separating them into multiple smaller lessons. The reason is that rendering even a basic triangle with modern OpenGL requires several components to work together.

We will cover:

1. Triangles and vertex data
2. The basic rendering process
3. Vertex Buffer Objects (VBOs)
4. Vertex Array Objects (VAOs)
5. Vertex and fragment shaders
6. Shader compilation and linking
7. The main rendering loop
8. Draw calls

These concepts form some of the most important building blocks of modern OpenGL rendering.

By the end of this lesson, we will be able to render a single triangle inside the GLFW window created in Lesson 1.

![Triangle](assets/triangle.png)

---

## 1. Triangle

### Triangle in Rendering

A triangle is one of the fundamental primitives used in computer graphics.

Geometrically, a triangle consists of three vertices connected by three edges. In rendering, however, each vertex can carry additional information beyond its position.

A vertex may contain attributes such as:

- position
- color
- normal
- texture coordinates
- tangent and bitangent vectors
- other application-defined data

For this lesson, each vertex will contain only two attributes:

- a **2D position**
- an **RGBA color**

Normals and other vertex attributes will become important in later lessons.

A **normal** is a vector perpendicular to a surface. Normals are particularly important for lighting calculations because they allow us to determine the orientation of a surface relative to a light source.

![Normal](assets/normal_visualization.png)

Normals are not required for the triangle rendered in this lesson, so they will not be included in our vertex data yet.

---

### Triangle Data

To render a triangle, we first need to describe its vertices.

The raw data used in this lesson looks like this:

```C++
float triangle_vertices[] = {
     0.0f,  0.5f,  0.0f, 1.0f, 1.0f, 1.0f,
    -0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 1.0f
};
```

Without knowing the structure of the array, these values do not communicate much information.

We can format the same data in a more readable way:

```C++
float triangle_vertices[] = {
    // position      // color
     0.0f,  0.5f,    0.0f, 1.0f, 1.0f, 1.0f, // top
    -0.5f, -0.5f,    0.0f, 1.0f, 1.0f, 1.0f, // bottom-left
     0.5f, -0.5f,    0.0f, 1.0f, 1.0f, 1.0f  // bottom-right
};
```

The array contains three vertices.

Each vertex contains six floating-point values:

    Vertex
    │
    ├── Position
    │   ├── X
    │   └── Y
    │
    └── Color
        ├── R
        ├── G
        ├── B
        └── A

Therefore, the memory layout is:

    X Y R G B A | X Y R G B A | X Y R G B A
    └─Vertex 0─┘   └─Vertex 1─┘   └─Vertex 2─┘

The order of the attributes is not inherently defined by OpenGL. We could store color before position, add normals between them, or use a completely different layout.

What matters is that we later tell OpenGL exactly how this memory should be interpreted.

---

### Positions

The three vertex positions are:

    Top:          [ 0.0,  0.5]
    Bottom-left:  [-0.5, -0.5]
    Bottom-right: [ 0.5, -0.5]

![Triangle with coordinates](assets/triangle_with_coords.png)

For this example, the coordinates are already specified in a range suitable for the final normalized coordinate system used by OpenGL.

The center of the screen corresponds to:

    [0.0, 0.0]

while the visible normalized range extends approximately from:

    [-1.0, 1.0]

along each axis after the perspective division stage.

Later lessons will introduce additional coordinate systems such as:

- local space
- world space
- view space
- clip space
- normalized device coordinates

For now, our vertex positions are deliberately chosen so that no transformations are necessary.

---

### Colors

Each vertex also contains four color components:

    R G B A

where:

- `R` — red
- `G` — green
- `B` — blue
- `A` — alpha

For normalized floating-point color values, each component typically uses the range:

    [0.0, 1.0]

For example:

    0.0f, 1.0f, 1.0f, 1.0f

represents a fully opaque cyan color.

The alpha component is commonly used when working with transparency and blending, which will be discussed in a later lesson.

---

## 2. Rendering Steps

Before examining each OpenGL object individually, it is useful to understand the complete process at a high level.

Ignoring the GLFW and OpenGL initialization covered in Lesson 1, rendering our triangle requires approximately the following steps:

1. Define the triangle vertex data.
2. Generate a Vertex Buffer Object (VBO).
3. Generate a Vertex Array Object (VAO).
4. Bind the VAO and VBO.
5. Upload the vertex data.
6. Describe the vertex memory layout.
7. Compile the vertex and fragment shaders.
8. Link the shaders into a shader program.
9. Enter the rendering loop.
10. Activate the shader program.
11. Bind the VAO.
12. Issue a draw call.
13. Swap the window buffers.
14. Process window events.

At a high level:

    CPU Vertex Data
        │
        ▼
        VBO
        │
        │  stores vertex data
        ▼
        VAO
        │
        │  describes how vertex data is read
        ▼
    Vertex Shader
        │
        ▼
    Rasterization
        │
        ▼
    Fragment Shader
        │
        ▼
    Framebuffer
        │
        ▼
    Window

This is intentionally a simplified representation of the OpenGL rendering pipeline.

The following sections examine each part individually.

---

## 3. Vertex Buffer Object — VBO

A **Vertex Buffer Object**, commonly abbreviated as **VBO**, is an OpenGL buffer object typically used to store vertex data.

Our triangle currently exists as a regular C++ array:

```C++
float triangle_vertices[] = {
    // ...
};
```

This data exists in application memory.

For OpenGL to efficiently use it during rendering, we create a buffer object and provide the vertex data to OpenGL.

An important distinction is that OpenGL buffer objects should not simply be understood as direct pointers into VRAM.

The OpenGL implementation manages the actual storage location. Depending on the implementation and usage, the underlying storage may reside in GPU-local memory, system memory, or be migrated between memory regions.

From the application's perspective, we work with an OpenGL **buffer object** identified by an integer handle.

---

### Creating a VBO

The VBO for our triangle is created using:

```C++
unsigned int vbo;

glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);

glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(triangle_vertices),
    triangle_vertices,
    GL_STATIC_DRAW
);
```

Let's examine each operation.

#### 1. Declare the Handle

```C++
unsigned int vbo;
```

This creates a regular C++ variable.

At this point, it does not refer to an OpenGL buffer object.

---

#### 2. Generate the Buffer Object Name

```C++
glGenBuffers(1, &vbo);
```

`glGenBuffers()` generates an unused OpenGL buffer object name and stores it in `vbo`.

The value can be thought of as a **handle** used to identify the buffer object.

It is not:

- a pointer to GPU memory
- the vertex data itself
- a C++ object containing the buffer

It is simply an identifier used by OpenGL.

---

#### 3. Bind the Buffer

```C++
glBindBuffer(GL_ARRAY_BUFFER, vbo);
```

OpenGL uses a binding-based state model.

This call binds `vbo` to the `GL_ARRAY_BUFFER` binding point.

A useful simplified mental model is:

    GL_ARRAY_BUFFER binding = vbo

Subsequent operations targeting `GL_ARRAY_BUFFER` therefore operate on the buffer currently bound to that target.

---

#### 4. Upload the Data

```C++
glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(triangle_vertices),
    triangle_vertices,
    GL_STATIC_DRAW
);
```

`glBufferData()` creates the buffer object's data store and initializes it with the contents of `triangle_vertices`.

Its arguments specify:

    GL_ARRAY_BUFFER
        │
        └── Which currently bound buffer should be modified?

    sizeof(triangle_vertices)
        │
        └── How many bytes should the buffer contain?

    triangle_vertices
        │
        └── Where should the initial data come from?

    GL_STATIC_DRAW
        │
        └── Usage hint describing the expected access pattern.

`GL_STATIC_DRAW` indicates that the data is expected to be specified relatively infrequently and used for drawing many times.

It is a usage **hint** to the OpenGL implementation rather than a strict storage requirement.

---

### Understanding Buffer Objects

A useful simplified analogy is a bank account:

    glGenBuffers()
        │
        └── Obtain an account number

    glBindBuffer()
        │
        └── Select the account

    glBufferData()
        │
        └── Deposit data into the selected account

The analogy is not technically exact, but it demonstrates an important aspect of OpenGL's state model:

> Many OpenGL operations affect whichever object is currently bound to a particular binding point.

This pattern appears repeatedly throughout OpenGL.

---

## 4. Vertex Array Object — VAO

At this stage, OpenGL has access to our vertex data.

However, the data is still simply a sequence of bytes.

Consider the raw array again:

```C++
float triangle_vertices[] = {
     0.0f,  0.5f,  0.0f, 1.0f, 1.0f, 1.0f,
    -0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 1.0f
};
```

We know that the first two numbers of every vertex represent its position and the next four represent its color.

OpenGL does not infer this meaning from the data.

We must explicitly describe the vertex layout.

This is where the **Vertex Array Object (VAO)** becomes important.

A VAO stores vertex input state, including the configuration used to interpret vertex attributes and the buffer associations captured by that configuration.

For our example, it describes how OpenGL should retrieve:

    Attribute 0 → Position
    Attribute 1 → Color

from the vertex data.

---

### Creating a VAO

The VAO is created and configured using:

```C++
unsigned int vao;

glGenVertexArrays(1, &vao);
glBindVertexArray(vao);

glVertexAttribPointer(
    0,
    2,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    reinterpret_cast<void*>(0)
);

glEnableVertexAttribArray(0);

glVertexAttribPointer(
    1,
    4,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    reinterpret_cast<void*>(2 * sizeof(float))
);

glEnableVertexAttribArray(1);
```

---

#### 1. Declare the VAO Handle

```C++
unsigned int vao;
```

As with the VBO, this initially creates only a C++ variable.

---

#### 2. Generate a VAO Name

```C++
glGenVertexArrays(1, &vao);
```

This generates an OpenGL vertex array object name and stores it in `vao`.

---

#### 3. Bind the VAO

```C++
glBindVertexArray(vao);
```

The VAO becomes the currently bound vertex array object.

Vertex attribute configuration performed afterward is stored as part of this VAO's state.

---

### Describing Vertex Attributes

Our vertex layout looks like this:

![Vertex data](assets/vetices_data.drawio.png)

Each vertex contains:

    ┌──────────────┬───────────────────────────┐
    │ Position     │ Color                     │
    ├──────┬───────┼──────┬──────┬──────┬──────┤
    │  X   │   Y   │  R   │  G   │  B   │  A   │
    └──────┴───────┴──────┴──────┴──────┴──────┘

That gives us a total of:

    6 floats per vertex

The position attribute is configured using:

```C++
glVertexAttribPointer(
    0,
    2,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    reinterpret_cast<void*>(0)
);
```

Let's examine each argument.

#### Attribute Index

```C++
0
```

This configures vertex attribute location `0`.

Later, the vertex shader will consume this attribute using:

```glsl
layout (location = 0) in vec2 a_pos;
```

---

#### Component Count

```C++
2
```

The position consists of two components:

    X, Y

Therefore, the shader receives the attribute as a `vec2`.

---

#### Component Type

```C++
GL_FLOAT
```

Each component is stored as a floating-point value.

---

#### Normalization

```C++
GL_FALSE
```

This determines whether certain fixed-point attribute data should be normalized when converted for shader input.

Our data is already stored as floating-point values, so normalization is not required.

---

#### Stride

```C++
6 * sizeof(float)
```

The **stride** specifies the byte distance between consecutive instances of the same vertex attribute.

Our layout is:

    X Y R G B A | X Y R G B A | X Y R G B A
    ^             ^
    |             |
    position 0    next position

There are six floats between the beginning of one vertex and the beginning of the next.

Therefore:

```C++
6 * sizeof(float)
```

is the stride.

Assuming a 4-byte `float`, this corresponds to:

    6 × 4 = 24 bytes

---

#### Offset

```C++
reinterpret_cast<void*>(0)
```

The position attribute begins at the first byte of each vertex.

Its offset is therefore:

    0 bytes

---

The attribute is then enabled:

```C++
glEnableVertexAttribArray(0);
```

Without enabling the attribute array, the vertex shader would not receive values from the configured vertex buffer for this attribute.

---

The color attribute is configured similarly:

```C++
glVertexAttribPointer(
    1,
    4,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    reinterpret_cast<void*>(2 * sizeof(float))
);

glEnableVertexAttribArray(1);
```

The differences are:

    Attribute location: 1
    Components:         4
    Type:               float
    Stride:             6 floats
    Offset:             2 floats

The offset is two floats because the first two values belong to the position:

    X Y R G B A
        ^
        |
        Color begins here

Assuming a 4-byte `float`:

    2 × 4 = 8 bytes

Therefore, the color attribute begins eight bytes after the beginning of each vertex.

![Vertex attribute indications](assets/vertez_attribute_indications.png)

At this stage, OpenGL knows that each vertex contains two attributes.

It does not inherently know that attribute `0` represents a position or that attribute `1` represents a color.

Those meanings are established by how the shader consumes and uses the attributes.

---

## 5. Vertex and Fragment Shaders

The next component required to render the triangle is a **shader program**.

### What Is a Shader?

A shader is a program executed as part of the graphics pipeline.

Shaders are written in a specialized shading language. In OpenGL, we use **GLSL — OpenGL Shading Language**.

Shaders are not specifically related to shadows despite the similarity in terminology.

They are general-purpose programmable stages of the graphics pipeline and can be used for:

- transforming vertices
- calculating colors
- texture sampling
- lighting
- animation
- material calculations
- post-processing
- many other rendering operations

For this lesson, we need two shader stages:

    Vertex Shader
        │
        ▼
    Rasterization
        │
        ▼
    Fragment Shader

---

### Vertex Shader

A **vertex shader** processes vertex shader invocations associated with the vertices submitted by the draw call.

For our triangle, three vertices are submitted:

    Vertex 0 ──► Vertex Shader
    Vertex 1 ──► Vertex Shader
    Vertex 2 ──► Vertex Shader

Our vertex shader is:

```glsl
#version 460 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;

out vec4 our_color;

void main()
{
    gl_Position = vec4(a_pos, 0.0, 1.0);
    our_color = a_color;
}
```

Its two primary responsibilities in this example are:

1. Produce the clip-space position of each vertex.
2. Pass the vertex color to later pipeline stages.

---

### Fragment Shader

After vertex processing and primitive rasterization, OpenGL generates **fragments**.

A fragment can be understood as a candidate contribution to a framebuffer pixel.

A fragment contains more information than simply a final pixel color. It may contain information such as:

- interpolated shader inputs
- depth
- coverage information
- other values used by later pipeline operations

The fragment shader processes these fragments.

Our fragment shader is:

```glsl
#version 460 core

in vec4 our_color;

out vec4 FragColor;

void main()
{
    FragColor = our_color;
}
```

The shader receives an interpolated color and outputs a color for the fragment.

For example, if the vertices have different colors, values can be interpolated across the surface of the triangle before being provided to fragment shader invocations.

The simplified pipeline is therefore:

    Vertex Data
        │
        ▼
    Vertex Shader
        │
        ▼
    Primitive Assembly
        │
        ▼
    Rasterization
        │
        ▼
    Fragments
        │
        ▼
    Fragment Shader
        │
        ▼
    Per-fragment Operations
        │
        ▼
    Framebuffer

Not every fragment necessarily becomes a visible pixel.

Later operations such as:

- depth testing
- stencil testing
- blending
- scissor testing

can affect the final framebuffer result.

These stages will be covered in later lessons.

---

### Vertex Shader Breakdown

Let's examine the vertex shader line by line.

```glsl
#version 460 core
```

This tells the GLSL compiler which language version the shader expects.

---

```glsl
layout (location = 0) in vec2 a_pos;
```

This declares an input variable at attribute location `0`.

Earlier, we configured:

```C++
glVertexAttribPointer(
    0,
    2,
    GL_FLOAT,
    GL_FALSE,
    6 * sizeof(float),
    reinterpret_cast<void*>(0)
);
```

The location numbers connect these two pieces of configuration:

    VAO Attribute 0
        │
        ▼
    layout(location = 0)
        │
        ▼
        a_pos

Because the VAO describes attribute `0` as two floating-point components, the shader receives it as:

```glsl
vec2
```

---

```glsl
layout (location = 1) in vec4 a_color;
```

Similarly:

    VAO Attribute 1
        │
        ▼
    layout(location = 1)
        │
        ▼
    a_color

The VAO describes this attribute as four floating-point components, so the shader receives a `vec4`.

---

```glsl
out vec4 our_color;
```

This declares an output from the vertex shader.

The value will be made available to the next relevant pipeline stage and eventually becomes an interpolated input to the fragment shader.

---

```glsl
void main()
```

As in C++, `main()` is the shader's entry point.

---

```glsl
gl_Position = vec4(a_pos, 0.0, 1.0);
```

`gl_Position` is a built-in GLSL output representing the final **clip-space position** of the vertex.

Our input position contains only:

    X, Y

but `gl_Position` requires four components:

    X, Y, Z, W

We therefore construct:

```glsl
vec4(a_pos, 0.0, 1.0)
```

which produces:

    X = a_pos.x
    Y = a_pos.y
    Z = 0.0
    W = 1.0

Clip-space coordinates are evaluated against the clipping volume.

Conceptually, for the conventional OpenGL clip volume:

    -w ≤ x ≤ w
    -w ≤ y ≤ w
    -w ≤ z ≤ w

After clipping, perspective division converts clip-space coordinates into normalized device coordinates:

    x_ndc = x / w
    y_ndc = y / w
    z_ndc = z / w

Because our `w` value is `1.0`, the input X and Y values remain unchanged after this division.

Later lessons will explore coordinate transformations in significantly more detail.

---

```glsl
our_color = a_color;
```

The vertex color received from the VBO is assigned to the vertex shader output.

---

### Fragment Shader Breakdown

The fragment shader begins with:

```glsl
#version 460 core
```

Again, this specifies the GLSL version.

---

```glsl
in vec4 our_color;
```

This corresponds to:

```glsl
out vec4 our_color;
```

from the vertex shader.

Conceptually:

    Vertex Shader

    out vec4 our_color;
            │
            ▼
    Rasterization
            │
            │ interpolation
            ▼
    Fragment Shader

    in vec4 our_color;

The value received by the fragment shader is generally interpolated across the triangle.

---

```glsl
out vec4 FragColor;
```

This declares the fragment shader's color output.

---

```glsl
void main()
{
    FragColor = our_color;
}
```

The fragment shader simply writes the interpolated color to its output.

This is intentionally a very simple shader. Later shaders will perform substantially more work, including lighting, texture sampling, material calculations, and other effects.

---

### Creating a Shader Program

Individual shader objects cannot simply be used independently for rendering.

They must be linked together into a **shader program**.

For this lesson, the program consists of:

    Vertex Shader
        +
    Fragment Shader
        │
        ▼
    Shader Program

The helper function used to create the program is:

```C++
unsigned int create_shader_program(
    const std::string& vertex_source,
    const std::string& fragment_source
)
{
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);

    if (!compile_shader(vertex_source, vertex_shader))
    {
        glDeleteShader(vertex_shader);
        return 0;
    }

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    if (!compile_shader(fragment_source, fragment_shader))
    {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }

    unsigned int shader_program = glCreateProgram();

    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    glLinkProgram(shader_program);

    int success;
    char info_log[512];

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(
            shader_program,
            512,
            nullptr,
            info_log
        );

        spdlog::error(
            "Shader program linking failed: {}",
            info_log
        );

        glDeleteProgram(shader_program);
        shader_program = 0;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}
```

The process consists of several stages.

#### 1. Create the Vertex Shader Object

```C++
unsigned int vertex_shader =
    glCreateShader(GL_VERTEX_SHADER);
```

This creates an OpenGL shader object intended to contain a vertex shader.

---

#### 2. Compile the Vertex Shader

```C++
if (!compile_shader(vertex_source, vertex_shader))
```

The GLSL vertex shader source is provided to OpenGL and compiled.

If compilation fails, the shader object is deleted and program creation stops.

---

#### 3. Create and Compile the Fragment Shader

```C++
unsigned int fragment_shader =
    glCreateShader(GL_FRAGMENT_SHADER);
```

followed by:

```C++
compile_shader(fragment_source, fragment_shader);
```

This performs the equivalent process for the fragment shader.

---

#### 4. Create the Program Object

```C++
unsigned int shader_program = glCreateProgram();
```

This creates an empty OpenGL program object.

A **shader object** and a **program object** are different OpenGL objects.

The shader objects contain compiled shader stages.

The program combines compatible shader stages into a linked executable used by the graphics pipeline.

---

#### 5. Attach the Shaders

```C++
glAttachShader(shader_program, vertex_shader);
glAttachShader(shader_program, fragment_shader);
```

Both compiled shader objects are attached to the program.

---

#### 6. Link the Program

```C++
glLinkProgram(shader_program);
```

Linking combines the attached shader stages into a program that can be used for rendering.

The linker also validates the interfaces between stages.

For example:

```glsl
// Vertex shader
out vec4 our_color;
```

must be compatible with:

```glsl
// Fragment shader
in vec4 our_color;
```

---

#### 7. Check the Link Status

```C++
glGetProgramiv(
    shader_program,
    GL_LINK_STATUS,
    &success
);
```

This queries whether program linking succeeded.

If it failed, the information log can be retrieved using:

```C++
glGetProgramInfoLog(
    shader_program,
    512,
    nullptr,
    info_log
);
```

This log contains diagnostic information produced by the linker.

---

#### 8. Delete the Individual Shader Objects

```C++
glDeleteShader(vertex_shader);
glDeleteShader(fragment_shader);
```

Once the program has been successfully linked, the individual shader objects can be deleted.

The linked program retains the executable information it needs.

The lifecycle can therefore be summarized as:

    Vertex GLSL Source
        │
        ▼
    Vertex Shader Object
        │
        │ compile
        ▼
    Compiled Vertex Shader ─────┐
                                │
                                ▼
                        Program Object
                                │
                                │ link
                                ▼
                        Shader Program
                                ▲
                                │
    Compiled Fragment Shader ───┘
        ▲
        │ compile
        │
    Fragment Shader Object
        ▲
        │
    Fragment GLSL Source

---

### Shader Compilation

The helper responsible for compiling an individual shader is:

```C++
int compile_shader(
    const std::string& source,
    unsigned int shader_id
)
{
    const char* source_c_str = source.c_str();

    glShaderSource(
        shader_id,
        1,
        &source_c_str,
        nullptr
    );

    glCompileShader(shader_id);

    int success;
    char info_log[512];

    glGetShaderiv(
        shader_id,
        GL_COMPILE_STATUS,
        &success
    );

    if (!success)
    {
        glGetShaderInfoLog(
            shader_id,
            512,
            nullptr,
            info_log
        );

        spdlog::error(
            "Shader compilation failed: {}",
            info_log
        );
    }

    return success;
}
```

The important shader-specific operations are:

---

#### `glShaderSource()`

```C++
glShaderSource(
    shader_id,
    1,
    &source_c_str,
    nullptr
);
```

This provides the GLSL source code to the shader object.

Conceptually:

    Shader Object
        │
        ◄── GLSL Source

---

#### `glCompileShader()`

```C++
glCompileShader(shader_id);
```

This requests compilation of the shader source associated with the shader object.

---

#### `glGetShaderiv()`

```C++
glGetShaderiv(
    shader_id,
    GL_COMPILE_STATUS,
    &success
);
```

This queries whether compilation succeeded.

---

#### `glGetShaderInfoLog()`

```C++
glGetShaderInfoLog(
    shader_id,
    512,
    nullptr,
    info_log
);
```

If compilation fails, this retrieves the shader compiler's diagnostic log.

The complete shader creation process is therefore:

    GLSL Source
        │
        ▼
    glShaderSource()
        │
        ▼
    Shader Object
        │
        ▼
    glCompileShader()
        │
        ▼
    Compiled Shader
        │
        ▼
    glAttachShader()
        │
        ▼
    Program Object
        │
        ▼
    glLinkProgram()
        │
        ▼
    Shader Program

---

## 6. Main Rendering Loop

At this stage, we have everything required to render the triangle:

- vertex data
- a VBO containing the data
- a VAO describing the vertex layout
- a vertex shader
- a fragment shader
- a linked shader program

We can now use these objects inside the main rendering loop.

The fundamental operations are:

```C++
while (!glfwWindowShouldClose(p_window))
{
    glClearColor(
        BACKGROUND_COLOR[0],
        BACKGROUND_COLOR[1],
        BACKGROUND_COLOR[2],
        BACKGROUND_COLOR[3]
    );

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glBindVertexArray(vao);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(p_window);
    glfwPollEvents();
}
```

Each frame can be summarized as:

    Clear Framebuffer
        │
        ▼
    Activate Shader Program
        │
        ▼
    Bind VAO
        │
        ▼
    Issue Draw Call
        │
        ▼
    Swap Buffers
        │
        ▼
    Process Events
        │
        └──────────────┐
                        │
                        ▼
                    Next Frame

---

### Clearing the Framebuffer

Lesson 1 introduced:

```C++
glClearColor(...);
glClear(GL_COLOR_BUFFER_BIT);
```

We can now explain why these operations are necessary.

Rendering does not directly modify the physical display.

OpenGL renders into a **framebuffer**.

A framebuffer contains attachments that can store rendering results such as:

- color
- depth
- stencil information

For this lesson, we are concerned only with the color buffer.

Suppose a triangle moves across the screen.

At frame `N`, a particular pixel may be covered by the triangle:

    Frame N

    Background Background Triangle Background

At frame `N + 1`, the triangle may have moved:

    Frame N + 1

    Background Background Background Triangle

If the color buffer were not cleared and nothing else wrote a new value to the old location, data from the previous frame could remain.

This could produce visual trails rather than a clean moving object.

Therefore, before rendering a new frame, we clear the relevant framebuffer attachments.

---

The clear color is specified using:

```C++
glClearColor(
    BACKGROUND_COLOR[0],
    BACKGROUND_COLOR[1],
    BACKGROUND_COLOR[2],
    BACKGROUND_COLOR[3]
);
```

For example:

```C++
constexpr float BACKGROUND_COLOR[4] = {
    0.1f,
    0.2f,
    0.3f,
    1.0f
};
```

`glClearColor()` specifies which RGBA value should be used when clearing color buffers.

It does not itself clear anything.

The actual clearing operation is:

```C++
glClear(GL_COLOR_BUFFER_BIT);
```

`GL_COLOR_BUFFER_BIT` tells OpenGL to clear the color buffer using the color previously specified by `glClearColor()`.

Conceptually:

    Previous Frame
        │
        ▼
    glClearColor()
        │
        │ defines clear value
        ▼
    glClear(GL_COLOR_BUFFER_BIT)
        │
        ▼
    Clean Color Buffer
        │
        ▼
    Render New Frame

---

### Issuing a Draw Call

Before issuing the draw call, we need to make the required rendering state active.

First:

```C++
glUseProgram(shader_program);
```

This makes `shader_program` the active shader program.

Subsequent rendering commands will use its shader stages.

---

Next:

```C++
glBindVertexArray(vao);
```

This binds the VAO containing our vertex input configuration.

At this point, OpenGL has the essential state required for the draw call:

    Shader Program
        │
        ├── How should vertices be processed?
        └── How should fragments be processed?

    VAO
        │
        └── How should vertex data be fetched?

Finally:

```C++
glDrawArrays(GL_TRIANGLES, 0, 3);
```

This issues the draw call.

Its arguments are:

```C++
glDrawArrays(
    GL_TRIANGLES,
    0,
    3
);
```

---

#### `GL_TRIANGLES`

```C++
GL_TRIANGLES
```

specifies the primitive type.

OpenGL interprets each group of three vertices as an independent triangle:

    Vertices 0, 1, 2 → Triangle 0
    Vertices 3, 4, 5 → Triangle 1
    Vertices 6, 7, 8 → Triangle 2
    ...

Our buffer contains only three vertices, so one triangle is produced.

---

#### First Vertex

```C++
0
```

This tells OpenGL to begin with vertex index `0` in the enabled vertex arrays.

---

#### Vertex Count

```C++
3
```

This tells OpenGL to process three vertices.

Because `GL_TRIANGLES` requires three vertices per triangle:

    3 vertices / 3 vertices per triangle = 1 triangle

The draw call connects all of the state we configured earlier.

Conceptually:

                    glDrawArrays()
                        │
            ┌─────────────┼─────────────┐
            ▼             ▼             ▼
        VAO        Shader Program   Draw Parameters
            │             │             │
            ▼             ▼             ▼
    Vertex Layout    GPU Programs   GL_TRIANGLES
            │             │             │
            └─────────────┼─────────────┘
                        ▼
                Rendering Pipeline
                        │
                        ▼
                    Framebuffer

This single call causes the configured rendering pipeline to process the submitted vertices and produce fragments that may ultimately update the framebuffer.

---

## 7. Framebuffer Size Callback

There is one additional detail worth handling: window resizing.

GLFW allows us to register a framebuffer size callback:

```C++
void framebuffer_size_callback(
    GLFWwindow* window,
    int width,
    int height
)
{
    glViewport(0, 0, width, height);
}
```

The callback is registered using:

```C++
glfwSetFramebufferSizeCallback(
    p_window,
    framebuffer_size_callback
);
```

When the framebuffer size changes, GLFW invokes the callback with the new dimensions.

Inside the callback, we call:

```C++
glViewport(0, 0, width, height);
```

The OpenGL **viewport** defines how normalized device coordinates are mapped to framebuffer coordinates.

Conceptually:

    Normalized Device Coordinates
            │
            ▼
        glViewport()
            │
            ▼
    Framebuffer Coordinates

The GLFW window size, framebuffer size, and OpenGL viewport are related concepts, but they should not be treated as the same thing.

This distinction is particularly important on displays where the framebuffer resolution can differ from the logical window size.

If the framebuffer changes size while the viewport remains unchanged, OpenGL continues mapping its output using the old viewport dimensions.

Updating the viewport in the framebuffer size callback ensures that rendering follows the new framebuffer dimensions.

For this lesson, the callback is:

```C++
void framebuffer_size_callback(
    GLFWwindow* window,
    int width,
    int height
)
{
    glViewport(0, 0, width, height);
}
```

and is registered using:

```C++
glfwSetFramebufferSizeCallback(
    p_window,
    framebuffer_size_callback
);
```

Whether an application always wants the viewport to match the complete framebuffer depends on the rendering design. For a simple application like this one, however, updating it during framebuffer resize is the expected behavior.

---

## Conclusion

We can now render our first primitive using modern OpenGL.

Although the final result is only a triangle, the important part of this lesson is not the triangle itself.

The important part is the infrastructure required to produce it.

We started with ordinary C++ data:

```C++
float triangle_vertices[] = {
    // ...
};
```

and transformed it into a rendered image through several stages.

The complete high-level process is:

    1. Define Vertex Data
            │
            ▼
    2. Create VBO
            │
            ▼
    3. Upload Vertex Data
            │
            ▼
    4. Create VAO
            │
            ▼
    5. Describe Vertex Layout
            │
            ▼
    6. Compile Shaders
            │
            ▼
    7. Link Shader Program
            │
            ▼
    8. Enter Rendering Loop
            │
            ▼
    9. Clear Framebuffer
            │
            ▼
    10. Bind Rendering State
            │
            ▼
    11. Issue Draw Call
            │
            ▼
    12. Present Frame

The relationship between the major OpenGL objects introduced in this lesson can be summarized as:

    C++ Vertex Data
        │
        ▼
        VBO
        │
        │ stores vertex data
        ▼
        VAO
        │
        │ describes vertex input
        ▼
    Vertex Shader
        │
        │ processes vertices
        ▼
    Rasterization
        │
        │ generates fragments
        ▼
    Fragment Shader
        │
        │ calculates fragment outputs
        ▼
    Framebuffer
        │
        │ stores rendering results
        ▼
    glfwSwapBuffers()
        │
        ▼
        Window

The most important concepts from this lesson are:

- A **VBO** provides storage for vertex data managed by OpenGL.
- A **VAO** stores the vertex input configuration required to interpret that data.
- A **vertex shader** processes vertex shader invocations and must produce clip-space positions.
- **Rasterization** converts primitives into fragments.
- A **fragment shader** processes fragments and produces outputs such as color.
- A **shader program** combines linked shader stages into a program that can be used by the rendering pipeline.
- A **framebuffer** stores rendering results.
- A **draw call** tells OpenGL to process vertices using the currently configured rendering state.

A single triangle may appear simple, but the same fundamental mechanisms scale to considerably more complex scenes.

More advanced rendering systems still rely on the same core ideas:

    Data
    +
    Vertex Layout
    +
    Shaders
    +
    Pipeline State
    +
    Draw Calls
    =
    Rendered Output

Understanding how these components interact provides the foundation for the OpenGL concepts introduced in the following lessons.