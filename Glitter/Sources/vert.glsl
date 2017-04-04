#version 150

attribute vec3 position;
attribute vec4 color;

out vec4 Color;
out vec2 Texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
  Color = color;
  gl_Position = proj * view * model * vec4(position, 1.0);
}
