#version 460 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;

uniform mat3 u_transform;

out vec4 our_color;

void main()
{
    vec3 transformed = u_transform * vec3(a_pos, 1.0);
    gl_Position = vec4(transformed.xy, 0.0, 1.0);
    our_color = a_color;
}
