#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos; 
  
out vec4 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float height;
uniform vec4 standart_color;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    

    ourColor = standart_color * height / 20;

    
}  

#shader fragment
#version 330 core

out vec4 FragColor;

in vec4 ourColor;


void main()
{
    FragColor = ourColor;

}