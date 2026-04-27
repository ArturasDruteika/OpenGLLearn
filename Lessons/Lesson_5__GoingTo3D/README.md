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
### Camera

For me, the best way to think about camera is like an actual operator that is filming the scene. Imagine, that you have a scene which has some objects placed in it (like tables, chairs and etc.). Now the operator can film the scene from many positions, maybe from the left side, maybe from the right. The operator can film the scene from the bottom or the top side. In other words, the operator can film it from any direction and any distance. The camera produces the visual information (what it captured), thus allowing viewers to see what is happening in that scene. The different position of the camera, produces the different view of the scene.

The exact same principles apply for the camera term in rendering. Instead of a human carying that camera from place to place, the position is controlled by the user him/herself. 

The image below illustrates everything you need to know about the camera for this lesson:

![camera](Assets/camera.png)

Let's go 1 by 1 and explain all of these:

* __Position__ - the coordinates of the camera.
* __Forward (View Direction)__ - it is a vector, a direction the camera is looking to. Imagine that from a camera a straight vector is point. This vector points to the direction the is focused.
* __Up__ - a vector that is pointing to the up direction from the camera and that is perpendicular to the forward vector.
* __Right__ - a vector that is pointing to the right direction from the camera  and that is perpendicular to the forward vector.
* __Near Plane (Image Plane)__ - the closest distance the object can be seen. Anything closer than that, and the object is not going to be rendered.
* __Far Plane__ the farthest distance the object can be seen. Anything farther than that, and the object is not going to be rendered.
* __View Frustum__ - a truncated pyramid (a pyramid with its top sliced off), where the two parallel faces are the near plane and the far plane.
* __Field of View (FOV)__ - How much of the scene the camera sees. Usually, the camera ___only___ everything that is inside the view frustum. Anything outside the view frustum is clipped.

Looking at the image, only 2 out of the 3 objects would be seen, the green cube and the red sphere, the blue cone would not be visible to the camera because it is outside the view frustum. 

Also, take a look at the __image plane__ in the image. In the parenthesis it is named as "screen". If it is easier for you, you can totally think of it like that, but please do not make a mistake thinking that the the view starts at the camera position. Now, why is that a problem? Well, mathematically, if a near plane is the coords of the camera, then the view frustum is not a frustum at all, but a pyramid, since no plane can be constructed in 1 point. That is why it is very important to have a near plane a little farther from the actual camera position, thus making a frustum possible.

#### Up and Right Vectors

Let's dive a little deeper for the __up__ and the __right__ vectors. First of all, why do we need these 2 vectors?

* __Up__ (camera up, not the world up) is used for the generation of the __view__ matrix (we are going to cover this later in this lesson).
* __Right__ is used to calculate the up vector.

So in a sense, we need A to construct C, but we need B to construct A, where A is up, B is right, C is view. 

How to calculate these vectors? Below are the equations for vectors caclulation:

$
    \mathbf{Right} = \mathrm{normalize}(\mathbf{Forward} \times \mathbf{WorldUp})
$

$
    \mathbf{Up} = \mathbf{Right} \times \mathbf{Forward}
$

Keep in mind 2 things:

1. `WorldUp` is a global up direction for the world, for example in 3D world, `up` vector would be [0, 1, 0] 
2. `x` is not a dot product but a cross section multiplication which is done this way:

    $
        \mathbf{a} =
        \begin{bmatrix} 
            a_x \\ a_y \\ a_z
        \end{bmatrix}, \quad
        \mathbf{b} =
        \begin{bmatrix} 
            b_x \\ b_y \\ b_z
        \end{bmatrix}
        \rightarrow
        \mathbf{a} \times \mathbf{b}
            =
        \begin{bmatrix} 
            a_y b_z - a_z b_y \\ 
            a_z b_x - a_x b_z \\ 
            a_x b_y - a_y b_x
        \end{bmatrix}
    $

Cross multiplication of 2 vectors is great because it spits out a 3-rd vector which is perpendicular to `a` and `b` vectors. To better understand why a 3-rd vector is perpendicular, just take a look at this image:

![cross_product](Assets/cross_product.png)

It clearly shows that a produced `n` vector is perpendicular to both `a` and `b`, or in other words, it is perpendicular to a plane that is defines by a total span of vectors `a` and `b`. Also, another important thing to see in this image is the order of multiplication. The cross product is not __commutatve__. `a × b` and `b × a` produce vectors that point in opposite directions. The exact direction is determined by the right-hand rule. Right hand rule works by pointing your index finger to the direction the first vector is pointing and middle finger - to the direction the second vector points, then the direction of the 3-rd vector is determined by your thumb. Just take a look at the image below:

![cross_product](Assets/cross_section_right_hand_rule.png)

The important thing to remember from this sections is that we need `up` (camera up) in order to get the `view` matrix.


#### Near / Far Planes and View Frustum

Once these 3 concepts are understood, you will understand half of what you see on your screen and why do you see it as you do ( a couple smiling faces). Why only a half? Well, because there are still concepts like __view__ and __projection__ matrices (which, as I said, I will cover on this lesson). But those, in my opinion, should be discussed after near, far planes and view frustum sits wells in your head. So, let's start digesting it. 

![view_frustum](Assets/view_frustum.png)

I hope this image elaborates a little more as to what I mean by near, far planes and view frustum. You can think of a near plane as to where the monitor screen starts. Far plane is where the visibility ends, and what you see on your screen is inside the view frustum (the blue volume in the image). I explained on why the near plane cannot start at the coordinates of a camera, but I never told why does the far plane have to have a defined end. 

The far plane has to have a defined ending because otherwise an infinite view frustum would happen. This would be a huge problem for your GPU, because looking infinitely far back there might be objects that are in the frustum's volume, so in a sense, calculations on those objects would be done. Also, for the same GPU problem, it is advised not to make a far plane super far away. Do not forget, that the more objects are visible, the more computation is needed to process and render them. But, it is up to the programmer to find an optimal solution for the distance of the far plane. Some scenes do not need the far plane to be very far away, but others, like a scenery of trees or a huge field require it to be farther away.

Again, to sum this little section, just know that based on the near and the far planes, the view frustum is constructed, and everything that is seen through the monitor screen is inside the view frustum.

#### Field of View (FOV)

__FOV__ is the angle that determines how much of the world the camera can see at one. This parameter is needed once we start calculating the projection matrix (which we will cover in this lesson).

To illustrate what the FOV is, here is the first explanatory image:

![fov_1](Assets/fov_1.png)

As you can see in this image, field of view angles fo from wide to narrow. Each colored triangle (in 3D it would be a pyramid) shows what area is visible through the camera lense. Wide angle is great in capturing near objects, but each captured objects has less detail in a single frame, while narrow angle is great for having a great detail in far away objects, but instead of seeing many objects, a few or a single object is visible. It is essentially a tradeoff, and each case or each combination might require to look at the scene with a different angle. The image below captures this idea:

![fov_2](Assets/fov_2.png)

There is a single tree in an empty field. The top right image shows how this scene is captured using a wide angle. Almost the entire scene is captured, all the mountains and hills, all of the sky and grass, captured in a single image. We can say that we see about 90% of the actual scene. But, using this picture, if we wanted to see the individual details of the tree or a mountain or even a grass, we would see that it lacks pixels. There is not enough detail for each individual object. Here comes the the bottom right image. A single tree is captured. A lot of detail for an individual tree, you can easiluy see it's branches and almost individual leafs. But, not a lot of background. Instead of the full view of the actual scene, we see about 5% of it, only a single tree with a little background that surrounds it. But then again, it is up to the person that uses the camera to determine what he / she wants to capture.

Also, looking at the top right and top bottom images, one important thing to say is that both images have the same amount of total pixels.

The exact same FOV principles hold in the rendering world. If you have a camera that has a wide FOV angle, you will be able to see a lot of scene at once throuh your monitor screen, but each separate object might not have the most detail and vice versa with a narrow FOV angle.

The equation for FOV is the following:

$
    FOV = 2 \cdot \arctan\left(\frac{sensor\_size}{2 \cdot focal\_length}\right)
$

But keep in mind that FOV is a single angle, meaning it describes the angle only for a single dimension.
You can specialize for horizontal and vertical FOV:

$
    FOV_h = 2 \cdot \arctan\left(\frac{sensor\_width}{2 \cdot focal\_length}\right)
$

$
    FOV_v = 2 \cdot \arctan\left(\frac{sensor\_height}{2 \cdot focal\_length}\right)
$

But, these are the equations that help as determine the FOV in the real world. In redering, we usually directly set the FOV. Also, one important thing to note here is that FOV is usually set for the vertcal component. Why? Well, as you will see later, the horizontal component can be extracted knowing what is the aspect ratio.


#### Aspect Ratio

__Aspect Ratio__ tells the ration between the pixels in width and pixels in height. No magic, just easy to understand topic. This parameter, same as FOV, is needed to calculate the projection matrix. The equation for the aspect ratio is this:

$
aspect\_ratio = \frac{width}{height}
$

I hope this image explains everything to you about the aspect ratio.

![aspect_ratio](Assets/aspect_ratio.png)

Now, that we have went through all the "small" building blocks of the camera, let's go to the "big bolder" topics.


---
### Camera Transformations: View and Projection Matrices

These 2 remaining topics about the camera are, at least for me, kind of hard to understand. I mean, I remember when I first stumbled on these 2 words and was like "what is that?". Trust me when I say this, but it took me some time to get a good grip on what these things are. This is, probobly, the first time where the notion of "why do I see objects like I do in pure form?" comes to light. First of all, in this section stop thinking anything outside the real world. I mean when I will be explaining these 2 matrices, do not think about no OpenGL or shaders or any other thing related to pixel coloration. Instead, all the time think about the real world (virtual world).

Remember how to get the model matrix of each object, you have to follow these steps:

$
    \text{Local} \;\rightarrow\; \text{Scaling} \;\rightarrow\; \text{Rotation} \;\rightarrow\; \text{Translation} \;\rightarrow\; \text{Model}
$

Well, somewhat the same logic can be applie here:

$
    \text{Model} \rightarrow \text{View} \rightarrow \text{Projection} \rightarrow \text{NDC} \rightarrow \text{Screen}
$

Before moving any further, here you can see __NDC__ and __Screen__. I am going to cover these 2 in this or the following lessons. For now, just know that NDC (normalized device coordinate) is resolution or pixel invariant 2D coordinate system. It's range is `[-1, 1]` (both, for X and for Y axes). Screen is the actual pixel coordinates (also 2D, because screen is flat), meaning that when X and Y coordinates are given, they represent the actual pixel position on your monitor screen. 

We know what a model matrix is, but following it, we can also see our 2 main protagonists of this section. Since __view__ goes right after the model matrix, let's start with it.


#### View Transformation

__View Matrix__ is a matrix that transforms world space into camera (view) space. It transforms all objects in the scene in such a way that the camera becomes the origin of the coordinate system and looks along a fixed direction. To help visualize this, imagine a cube placed at position [x, y, z] in world space, and a camera located at `[x_c, y_c, z_c]`. After applying the view matrix, the coordinate system is transformed so that the camera is effectively at `[0, 0, 0]`, and the cube is moved and rotated to a new position `[x_t, y_t, z_t]` relative to the camera. In other words, instead of moving the camera, the entire world is transformed relative 
to the camera's position and orientation.

What might help you to better understand how a camera view coordinate are how the axis look like. Remember what are camera's forward direction, right and up vectors? Well, these become axes in the camera view coordinate system:

* Forward vector --> -Z axis (since Z axis goes towards the screen)
* Right vector --> +X axis
* Up vector --> +Y axis

The idea of view matrix is kinda simple. You need a view transform in order to move all the world in such a way that the camera would be the origin of it. You need the camera to be the origin because you see the real world through your monitor, and he monitor is eesentialy the camera lense through which you see the real world. 

There are also deeper and much more important reasons on why the view transform is necessary. For example, it mathematically simplifies a lot of the stuff for the GPU. Imagine for a second that you did not use the view transform. In this case, now the GPU has to somehow figure out what is visible and what is not. And how would the GPU do it? Well, in this case, for every object the GPU would have to calculate the camera orientation (because we want to see if the object is visible or not), compute projections differently. We would need all this stuff because, at the end, the clip space essentially tells what objects are in the camera view are and which not. View transformation simplifies a lot of steps, and also, because matrices are amazing, you can combine the view matrix with the projection matrix (which we are going to discuss now).

#### Projection Transformation

Dilemma is simple - we have 3D objects which are represented by triangles where each vertex is defined in 3D space (X, Y, Z) coordinates. The problem is that our screen is 2D, and pixels are usually specified by 2 coordinates [X, Y]. Now, how can we transform 3D points in a way that we could see them on a 2D screen? Lucky for us, we have one special transformation, suited exactly for these kinda situations. Here comes the __projection transformation__.

__Projection Transformation__ - transforms 3D points in camera (view) space into a form that can be mapped onto a 2D screen. 

Remember this image:

![camera](Assets/camera.png)

Well in this situation the view on your monitor should be something like this:

![camera](Assets/camera_view_through_monitor.png)