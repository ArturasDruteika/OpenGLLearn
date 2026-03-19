# Lesson 4 - Transformations

In this tesson we are going to learn tranformations and, as a side thing, shader uniforms. Transformations, as a whole is one of those building blocks, that someone, who is learning graphics, should definitely master it. But do not get too scared, unless you want to become a savant 3D graphics engineer, because transformations (at least to what I have used) is not a thing that a simple person, who does not understand math well, cannot understand.

Since I am not the best at explaining math topics and before continuing any further, I highly sugegst to you to:

* Read [this article](https://learnopengl.com/Getting-started/Transformations)
* If you are more of a visual learning, I advise you to look [this playlist](https://www.youtube.com/watch?v=fNk_zzaMoSs&list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab) by [3Blue1Brown](https://www.youtube.com/@3blue1brown/playlists). This, in my opinion, is the best way to visually understand the concepts of linear algebra.

Again, do not get discouraged by the fact than N years ago, during math classes you did not pay attention, but you need to know the gist of it.

As for the uniforms, it is also, one of the building blocks you have to understand, but it is as simple as learning what a shader or vertex buffer is.

---
### Transformation types

For the basic 3D rendering, in my opinion, you need to know 3 types of transformations:

1. __Scaling__ - resizing the object to make it bigger or smaller.
![Scaling](Assets/scaling.png)
2. __Rotation__ - rotating the object around one of it's axis.
![Rotation](Assets/rotation.png)
3. __Translation__ - moving an object in space from position A to position B.
![Translation](Assets/translation.png)

This is probably 99% of transformations you will need if you want to understand how rendering works. Again, I just want to stress this that this topic of __transformations__ is also an integral part if you want to understand how rendering works. I know, that it may sound weird, I mean "how does knowing how objects can be transformed help me to understand how to render an image". But without tranformations, all of your scenes will be just a static image where nothing happens. 

Also, keep in mind, that I just showed you how transformations happen in 2D. But do not get too afraid, in 3D, these transformations happen the same way, just that there are more axis on which we can transform objects.


### Code Part

