# Lesson 1 — Creating a Window with GLFW


## Contents

- [Overview](#overview)
- [Main GLFW Operations](#main-glfw-operations)
  - [1. Initialize GLFW](#1-initialize-glfw)
  - [2. Create the Window and OpenGL Context](#2-create-the-window-and-opengl-context)
  - [3. Make the OpenGL Context Current](#3-make-the-opengl-context-current)
  - [4. Load OpenGL Functions with GLAD](#4-load-opengl-functions-with-glad)
  - [5. Enter the Application Loop](#5-enter-the-application-loop)
  - [6. Clear the Framebuffer](#6-clear-the-framebuffer)
  - [7. Swap the Front and Back Buffers](#7-swap-the-front-and-back-buffers)
  - [8. Process Window Events](#8-process-window-events)
  - [9. Destroy the Window](#9-destroy-the-window)
  - [10. Terminate GLFW](#10-terminate-glfw)
- [Error Handling](#error-handling)
- [GLFW and OpenGL Responsibilities](#glfw-and-opengl-responsibilities)
- [Important Distinctions](#important-distinctions)
  - [GLFW Initialization vs. OpenGL Context Creation](#glfw-initialization-vs-opengl-context-creation)
  - [Context Creation vs. Making a Context Current](#context-creation-vs-making-a-context-current)
  - [Context Creation vs. OpenGL Function Loading](#context-creation-vs-opengl-function-loading)
- [Why Do We Need a Windowing Library?](#why-do-we-need-a-windowing-library)
- [Complete Example](#complete-example)
- [Conclusion](#conclusion)

---

This lesson introduces the minimum setup required to create a window with GLFW and establish an OpenGL context.

Most of the code in this example is concerned with **GLFW**. OpenGL functionality, accessed through GLAD, is intentionally limited to the following three operations:

```cpp
gladLoadGL(glfwGetProcAddress);

glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT);
```

The OpenGL-specific functionality will be covered in more detail in later lessons. For now, the focus is on understanding the role of GLFW, creating and activating an OpenGL context, and maintaining a basic application loop.

By the end of this lesson, we will have a minimal application capable of displaying a GLFW window.

![GLFW Window](assets/glfw_window.png)

---

## Main GLFW Operations

Creating a minimal GLFW window can be broken down into the following steps.

### 1. Initialize GLFW

```cpp
glfwInit();
```

`glfwInit()` initializes the GLFW library and the platform-specific resources required by it.

It is important to distinguish GLFW initialization from OpenGL initialization:

> `glfwInit()` initializes **GLFW**. It does not initialize OpenGL or create an OpenGL context.

At this point, no window or OpenGL context has been created.

---

### 2. Create the Window and OpenGL Context

```cpp
GLFWwindow* p_window = glfwCreateWindow(
    800,
    600,
    "GLFW Window",
    nullptr,
    nullptr
);
```

The function has the following signature:

```cpp
GLFWwindow* glfwCreateWindow(
    int width,
    int height,
    const char* title,
    GLFWmonitor* monitor,
    GLFWwindow* share
);
```

`glfwCreateWindow()` creates a platform-specific window and, unless configured otherwise, an associated OpenGL context.

Creating the context does not automatically make it current on the calling thread. Before OpenGL operations can be performed through that context, it must first be made current.

This distinction becomes important as the application grows:

- **Window** — the OS-managed area in which the rendered image is presented.
- **OpenGL context** — stores the OpenGL state associated with rendering.
- **Current context** — the context OpenGL commands issued by the current thread operate on.

---

### 3. Make the OpenGL Context Current

```cpp
glfwMakeContextCurrent(p_window);
```

`glfwMakeContextCurrent()` makes the OpenGL context associated with `p_window` current on the calling thread.

OpenGL commands operate on the context that is current for the thread executing them. Therefore, the context must be made current before loading OpenGL functions or issuing OpenGL commands.

The distinction between context creation and activation is important:

```text
glfwCreateWindow(...)
        │
        └── Creates window
        └── Creates associated OpenGL context
                        │
                        ▼
glfwMakeContextCurrent(...)
        │
        └── Makes that context current on this thread
```

In short:

- `glfwCreateWindow()` **creates** the OpenGL context.
- `glfwMakeContextCurrent()` makes that context **current on the calling thread**.

---

### 4. Load OpenGL Functions with GLAD

```cpp
gladLoadGL(glfwGetProcAddress);
```

After an OpenGL context has been created and made current, GLAD can load the OpenGL functions provided by the graphics driver.

GLAD is an **OpenGL function loader**. It provides access to OpenGL functions by obtaining their function pointers at runtime.

GLFW provides:

```cpp
glfwGetProcAddress
```

which GLAD uses to resolve the OpenGL function addresses for the current platform and context.

The initialization order is therefore significant:

```text
Initialize GLFW
      │
      ▼
Create window + OpenGL context
      │
      ▼
Make context current
      │
      ▼
Load OpenGL functions with GLAD
      │
      ▼
OpenGL commands can now be used
```

Calling OpenGL functionality exposed through the loader before the required function pointers have been loaded would result in invalid function access.

For this reason, GLAD initialization happens only after a valid OpenGL context has been made current.

---

### 5. Enter the Application Loop

```cpp
while (!glfwWindowShouldClose(p_window))
{
    // ...
}
```

Once initialization is complete, the application enters its main loop.

```cpp
glfwWindowShouldClose(p_window)
```

checks the close flag associated with the window.

The loop continues until that flag is set, typically when the user requests that the window be closed.

For this lesson, every iteration performs four basic operations:

```cpp
while (!glfwWindowShouldClose(p_window))
{
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(p_window);
    glfwPollEvents();
}
```

Conceptually, the loop performs:

```text
Clear / Render
      │
      ▼
Present Frame
      │
      ▼
Process Events
      │
      └──────────────┐
                     │
                     ▼
               Next Frame
```

This loop forms the foundation of an interactive graphical application. Later lessons will add actual rendering operations to it.

---

### 6. Clear the Framebuffer

```cpp
glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT);
```

These are the first actual OpenGL rendering operations used in the project.

`glClearColor()` specifies the color OpenGL should use when clearing the color buffer.

The four parameters represent:

```text
Red, Green, Blue, Alpha
```

with values typically expressed in the `[0.0, 1.0]` range.

In this example:

```cpp
glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
```

defines a dark blue background color.

The following call:

```cpp
glClear(GL_COLOR_BUFFER_BIT);
```

clears the framebuffer's color buffer using the previously specified clear color.

For now, these operations are sufficient to produce a visible result. Framebuffers and OpenGL rendering operations will be discussed in greater detail in later lessons.

---

### 7. Swap the Front and Back Buffers

```cpp
glfwSwapBuffers(p_window);
```

The GLFW window uses **double buffering**.

Instead of rendering directly into the image currently visible on the screen, rendering is normally performed into a **back buffer**.

Conceptually:

```text
Front Buffer                Back Buffer
(currently displayed)       (currently rendered)

┌───────────────┐           ┌───────────────┐
│               │           │               │
│ Previous      │           │ New           │
│ Frame         │           │ Frame         │
│               │           │               │
└───────────────┘           └───────────────┘
        ▲                           │
        │                           │
        └──── glfwSwapBuffers ──────┘
```

Once the new frame has been rendered, `glfwSwapBuffers()` exchanges the front and back buffers.

The newly rendered image becomes the image presented by the window.

This prevents the user from observing the framebuffer while it is still being constructed.

---

### 8. Process Window Events

```cpp
glfwPollEvents();
```

`glfwPollEvents()` processes pending events received from the operating system.

These events can include:

- keyboard input
- mouse movement
- mouse button input
- scroll input
- window resizing
- window close requests
- window focus changes
- window movement
- other platform-specific window events

Without event processing, the application would not properly respond to interaction with the operating system or user input.

A typical GLFW application therefore processes events once per iteration of the main loop.

---

### 9. Destroy the Window

```cpp
glfwDestroyWindow(p_window);
```

Once the main loop terminates, the GLFW window is no longer required.

`glfwDestroyWindow()` destroys the specified window and its associated context and releases the resources associated with them.

This is performed after the application exits the main loop:

```cpp
while (!glfwWindowShouldClose(p_window))
{
    // Application loop
}

glfwDestroyWindow(p_window);
```

---

### 10. Terminate GLFW

```cpp
glfwTerminate();
```

Finally, `glfwTerminate()` shuts down GLFW and releases resources allocated by the library.

It should be called once GLFW is no longer required.

The complete lifetime of the application can therefore be summarized as:

```text
glfwInit()
    │
    ▼
glfwCreateWindow()
    │
    ▼
glfwMakeContextCurrent()
    │
    ▼
gladLoadGL()
    │
    ▼
┌────────────────────────────┐
│       Application Loop     │
│                            │
│  Clear / Render            │
│  glfwSwapBuffers()         │
│  glfwPollEvents()          │
└────────────────────────────┘
    │
    ▼
glfwDestroyWindow()
    │
    ▼
glfwTerminate()
```

---

## Error Handling

Each initialization operation that can fail should be checked before continuing.

### GLFW Initialization

```cpp
if (!glfwInit())
{
    spdlog::error("Failed to initialize GLFW");
    return -1;
}
```

If GLFW cannot be initialized, the application cannot continue because all subsequent GLFW functionality depends on successful initialization.

### Window Creation

```cpp
GLFWwindow* p_window = glfwCreateWindow(
    800,
    600,
    "GLFW Window",
    nullptr,
    nullptr
);

if (!p_window)
{
    spdlog::error("Failed to create GLFW window");
    glfwTerminate();
    return -1;
}
```

If window creation fails, GLFW itself has already been initialized and must therefore be terminated before the application exits.

### GLAD Initialization

```cpp
if (!gladLoadGL(glfwGetProcAddress))
{
    spdlog::error("Failed to initialize GLAD");
    glfwDestroyWindow(p_window);
    glfwTerminate();
    return -1;
}
```

At this stage, both GLFW and the window have already been created.

If GLAD initialization fails, both resources must be released before terminating the application.

This produces a simple resource lifetime:

```text
Resource acquired           Resource released
──────────────────────────────────────────────

glfwInit()                  glfwTerminate()

glfwCreateWindow()          glfwDestroyWindow()

OpenGL context              destroyed with window
```

The general principle is:

> Resources should be released in the reverse order in which they were acquired.

This becomes increasingly important as more resources are introduced throughout the project.

---

## GLFW and OpenGL Responsibilities

GLFW and OpenGL solve different parts of the graphics application.

| Responsibility | GLFW | OpenGL |
|---|:---:|:---:|
| Initialize GLFW | Yes | No |
| Create an OS window | Yes | No |
| Create an OpenGL context | Yes | No |
| Make an OpenGL context current | Yes | No |
| Process window events | Yes | No |
| Handle keyboard and mouse events | Yes | No |
| Swap window buffers | Yes | No |
| Provide the rendering API | No | Yes |
| Manage graphics pipeline state | No | Yes |
| Execute drawing commands | No | Yes |
| Produce framebuffer contents | No | Yes |

GLAD has a separate responsibility:

| Responsibility | GLAD |
|---|:---:|
| Load OpenGL function pointers | Yes |
| Create a window | No |
| Create an OpenGL context | No |
| Implement OpenGL | No |
| Perform rendering | No |

A useful high-level model is therefore:

```text
Operating System
      │
      ▼
    GLFW
      │
      ├── Window creation
      ├── Input / events
      ├── OpenGL context creation
      └── Buffer presentation
             │
             ▼
        OpenGL Context
             │
             ▼
            GLAD
      (function loading)
             │
             ▼
          OpenGL
      (rendering API)
             │
             ▼
        Graphics Driver
             │
             ▼
            GPU
```

This is intentionally simplified, but it provides a useful mental model for the responsibilities of each component.

---

## Important Distinctions

### GLFW Initialization vs. OpenGL Context Creation

```cpp
glfwInit();
```

initializes the GLFW library.

It does **not** create an OpenGL context.

The context is created when the window is created:

```cpp
glfwCreateWindow(...);
```

---

### Context Creation vs. Making a Context Current

Creating a context and making it current are separate operations.

```cpp
GLFWwindow* p_window = glfwCreateWindow(...);
```

creates the window and its associated OpenGL context.

```cpp
glfwMakeContextCurrent(p_window);
```

makes that context current on the calling thread.

This distinction becomes especially relevant in applications that use multiple windows, multiple OpenGL contexts, or multiple threads.

---

### Context Creation vs. OpenGL Function Loading

Creating an OpenGL context does not mean that GLAD has loaded the OpenGL function pointers.

The required order is:

```cpp
glfwCreateWindow(...);
glfwMakeContextCurrent(p_window);
gladLoadGL(glfwGetProcAddress);
```

Only after this initialization sequence should OpenGL functionality exposed through GLAD be used.

---

## Why Do We Need a Windowing Library?

If OpenGL is responsible for rendering, why is GLFW necessary?

Is displaying a window not also part of rendering?

Not exactly.

OpenGL and the operating system's windowing system operate at different levels.

OpenGL is a **graphics API**. Its responsibility is to provide commands for producing graphical output. As the project progresses, this will include working with:

- geometry
- vertex buffers
- shaders
- textures
- framebuffers
- depth and stencil buffers
- blending
- graphics pipeline state

Ultimately, these operations contribute to the contents of a framebuffer.

A desktop window, however, is managed by the **operating system's windowing system**.

The windowing system is responsible for functionality such as:

- creating and destroying windows
- positioning windows on the desktop
- resizing windows
- handling focus
- receiving keyboard and mouse events
- interacting with the desktop compositor
- presenting application surfaces alongside other windows

OpenGL does not provide a cross-platform API for creating desktop windows or processing operating-system events.

This is where GLFW is used.

GLFW provides a cross-platform abstraction over the platform-specific windowing APIs and handles the setup required to create a window suitable for OpenGL rendering.

The relationship can be summarized as:

```text
               Operating System
                      │
                      ▼
              Windowing System
                      │
                      ▼
                    GLFW
                ┌─────┴─────┐
                │           │
                ▼           ▼
             Window     OpenGL Context
                │           │
                │           ▼
                │        OpenGL
                │           │
                │           ▼
                │       Framebuffer
                │           │
                └─────┬─────┘
                      ▼
                  Presentation
                      │
                      ▼
                    Screen
```

When GLFW creates the window, it also creates an OpenGL context associated with that window.

After the context has been made current and the required OpenGL functions have been loaded, OpenGL can render into the framebuffer associated with the context/window setup.

The rendered back buffer is then presented through:

```cpp
glfwSwapBuffers(p_window);
```

The operating system ultimately determines where that window appears and how its contents are composited with the rest of the desktop.

In short:

> **OpenGL produces graphical output. GLFW provides the window, context management, event handling, and presentation integration required to display that output in a desktop application.**

---

## Conclusion

This lesson establishes the basic infrastructure required before more substantial OpenGL rendering can begin.

The application now performs the following sequence:

1. Initializes GLFW.
2. Creates an operating-system window and an associated OpenGL context.
3. Makes the OpenGL context current on the application's thread.
4. Loads OpenGL function pointers using GLAD.
5. Enters an application loop.
6. Clears the framebuffer.
7. Presents the rendered buffer.
8. Processes operating-system events.
9. Destroys the window and its associated context.
10. Terminates GLFW.

The most important distinction from this lesson is the separation of responsibilities between the different components:

- **GLFW** manages the window, OpenGL context creation, events, and presentation.
- **GLAD** loads the OpenGL function pointers required by the application.
- **OpenGL** provides the graphics API used to produce the contents of the framebuffer.
- **The operating system** manages the desktop windowing environment and ultimately presents application windows on the display.

With the window, context, and application loop in place, the next lessons can focus on the actual OpenGL rendering pipeline.