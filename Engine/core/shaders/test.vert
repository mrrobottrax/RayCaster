#version 450
layout (location = 0) in vec3 aPos;
  
layout (location = 0) out vec4 vertexColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
}