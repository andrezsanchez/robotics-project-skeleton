#version 150

uniform float time;

in vec4 Color;
/*in vec3 Normal;*/

out vec4 outColor;

void main()
{
  outColor = Color;
}
