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
