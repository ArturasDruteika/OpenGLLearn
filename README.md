# OpenGL Learn

A learning-focused OpenGL repository that documents the process of building a renderer from the fundamentals toward a simple **3D solar system with realistic lighting**.

The repository is organized as a sequence of lessons. Each lesson introduces new rendering concepts, explains the underlying ideas, and then applies them in OpenGL.

The long-term goal is to gradually move from drawing the first triangle to understanding enough graphics programming to render a small 3D scene containing planets, lighting, cameras, transformations, and other features required by the project.

> **This repository is primarily a learning project, not an authoritative OpenGL reference.**
>
> I explain concepts the way I currently understand them and improve the material as my understanding develops. Although I try to keep the explanations and code accurate, mistakes or oversimplifications may still exist.

---

# Setup & Building
1. Clone the repository and initialize submodules:
    ```sh
    git clone --recursive https://github.com/ArturasDruteika/open_gl_learn.git
    # or, if already cloned:
    git submodule update --init --recursive
    ```
2. Install build dependencies (Linux example):
    ```sh
    ./scripts/setup-linux.sh
    ```
3. Build the project:
    ```sh
    ./scripts/build-linux.sh
    ```
4. Run the app (Example)
    ```sh
    cd build/release/bin
    ./name_of_the_lesson
    # e.g. ./lesson_5__going_to_3d
    ```

---

# Contents

- [About the Project](#about-the-project)
- [Lessons](#lessons)
  - [Lesson 1 — Introduction to OpenGL](lessons/lesson_1__glfw_window/README.md)
  - [Lesson 2 — Vertex Buffer Objects and Vertex Array Objects](lessons/lesson_2__triangle/README.md)
  - [Lesson 3 — Element Buffer Objects](lessons/lesson_3__ebo/README.md)
  - [Lesson 4 — Transformations](lessons/lesson_4__transformations/README.md)
  - [Lesson 5 — Going to 3D](lessons/lesson_5__going_to_3d/README.md)
- [Learning Path](#learning-path)
- [Repository Structure](#repository-structure)
- [Recommended Resources](#recommended-resources)
- [OpenGL Documentation](#opengl-documentation)
- [A Note About the Tutorials](#a-note-about-the-tutorials)

---

# About the Project

Learning graphics programming can be difficult because even rendering something simple requires understanding several systems at once.

A triangle on the screen eventually leads to questions about:

- vertex buffers
- vertex array objects
- index buffers
- shaders
- coordinate systems
- matrices
- transformations
- cameras
- perspective projection
- depth testing
- textures
- normals
- lighting
- materials
- and many other topics

The purpose of this repository is to learn these concepts **incrementally**.

Instead of immediately trying to build a complete renderer, each lesson adds another piece to the system.

The general progression is:

    Basic OpenGL
        ↓
    Vertex Data
        ↓
    Indexed Geometry
        ↓
    Transformations
        ↓
    3D Rendering
        ↓
    Cameras
        ↓
    Lighting
        ↓
    Materials
        ↓
    Solar System

The final destination is not as important as understanding the systems required to get there.

---

# Learning Path

The lessons are designed to build on one another.

At a high level:

| Lesson | Main Concept | Result |
|---|---|---|
| **Lesson 1** | OpenGL fundamentals | Render basic geometry |
| **Lesson 2** | VBOs and VAOs | Organize vertex data |
| **Lesson 3** | EBOs | Render indexed geometry |
| **Lesson 4** | Transformations | Move, rotate, and scale objects |
| **Lesson 5** | 3D rendering | Camera, perspective, and depth |

The concepts also build into one another:

    Vertex Data
        ↓
    VBO + VAO
        ↓
    Indexed Rendering
        ↓
    EBO
        ↓
    Model Transformations
        ↓
    World Space
        ↓
    Camera / View Matrix
        ↓
    Perspective Projection
        ↓
    3D Scene

Future lessons can continue extending this pipeline toward the final solar-system renderer.

---

# Repository Structure

The repository is organized around the lessons:

    OpenGL-Learn/
    │
    ├── README.md
    │
    ├── lessons/
    │   ├── lesson_1/
    │   │   └── README.md
    │   │
    │   ├── lesson_2/
    │   │   └── README.md
    │   │
    │   ├── lesson_3/
    │   │   └── README.md
    │   │
    │   ├── lesson_4/
    │   │   └── README.md
    │   │
    │   └── lesson_5/
    │       └── README.md
    │
    └── ...

Each lesson contains its own explanation of the concepts introduced at that stage of the project.

The repository should therefore be read approximately like a small course:

    README
        ↓
    Lesson 1
        ↓
    Lesson 2
        ↓
    Lesson 3
        ↓
    Lesson 4
        ↓
    Lesson 5
        ↓
    ...

---

# Recommended Resources

This repository documents **my own learning process**.

If your goal is to gain a deeper and more complete understanding of OpenGL and graphics programming, I strongly recommend using established learning resources alongside these lessons.

The following resources have had the biggest influence on my understanding of rendering and OpenGL.

---

## 1. The Cherno

[The Cherno — YouTube Channel](https://www.youtube.com/@TheCherno)

The Cherno has extensive content about C++, OpenGL, game engines, and graphics programming.

Two particularly useful series are:

- [OpenGL Series](https://www.youtube.com/watch?v=W3gAzLwfIP0&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2)
- [Game Engine Series](https://www.youtube.com/watch?v=JxIZbV_XjAs&list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT)

These videos were one of the resources that initially helped me understand how the different pieces of a renderer fit together.

---

## 2. LearnOpenGL

[LearnOpenGL](https://learnopengl.com/)

LearnOpenGL is one of the most useful resources for learning modern OpenGL from the beginning.

It covers topics such as:

- getting started with OpenGL
- shaders
- textures
- transformations
- coordinate systems
- cameras
- lighting
- model loading
- framebuffers
- advanced OpenGL
- physically based rendering

It is an excellent resource both for learning and for revisiting concepts later.

---

## 3. Learning Modern 3D Graphics Programming

[Learning Modern 3D Graphics Programming](https://paroj.github.io/gltut/)

This book provides a more detailed introduction to modern 3D graphics programming.

It is particularly useful when you want to go beyond simply learning which OpenGL functions to call and understand more of the reasoning behind the graphics pipeline.

---

# OpenGL Documentation

When working with OpenGL, you will frequently need to look up:

- function signatures
- parameter meanings
- accepted enum values
- OpenGL state behavior
- error conditions

A convenient reference is:

[docs.gl — OpenGL Documentation](https://docs.gl/)

For example, when encountering a function such as:

```cpp
glDrawElements(
    GL_TRIANGLES,
    6,
    GL_UNSIGNED_INT,
    nullptr
);
```

you should gradually become comfortable looking up the function and understanding exactly what each argument represents.

Learning how to navigate graphics API documentation is an important skill by itself.

---

# A Note About the Tutorials

I do not know exactly how far this tutorial series will go.

The current goal is to continue learning enough graphics programming to build increasingly interesting scenes and eventually render a simple **solar system with realistic lighting**.

These lessons are written while I am learning the subject myself.

Because of that, they should not be treated as a replacement for:

- official documentation
- established OpenGL books
- graphics-programming textbooks
- high-quality courses and tutorials

Instead, think of this repository as:

> **A documented journey through learning OpenGL and real-time graphics programming, with each lesson building another part of a renderer.**

I will try to explain concepts from first principles and in the way that made them understandable to me.

As my understanding improves, earlier lessons may also be corrected, expanded, or reorganized.

If you notice an error, an inaccurate explanation, or something that could be explained more clearly, corrections and suggestions are welcome.
