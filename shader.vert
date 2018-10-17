#version 330 core

layout(location=0) in vec2 window_pos;

void
main()
{
    gl_Position = vec4(window_pos.xy, 0.0, 1.0);
}
