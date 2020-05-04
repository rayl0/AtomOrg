#version 420 

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texcoords;

uniform mat3 proj;
uniform mat3 model;

out vec2 tex_coords;

void main()
{
   vec3 position = proj * model * vec3(pos, 1);
   gl_Position = vec4(position.xy, 0, 1.0f);
   tex_coords = texcoords;
}

!!!

#version 420

in vec2 tex_coords;
out vec4 color;

uniform sampler2D tex;

void main()
{
   color = texture(tex, tex_coords);
}
