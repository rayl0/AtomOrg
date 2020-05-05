#version 420

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texturecoords;

uniform mat3 model;
uniform mat3 proj;

out vec2 tex_coords;

void main()
{
    gl_Position = vec4(proj * model * vec3(pos, 1), 1);
    tex_coords = texturecoords;
}

!!!

#version 420

in vec2 tex_coords;

uniform vec3 color;
uniform float alpha;

uniform sampler2D ui_texture;
uniform float use_texture;

out vec4 out_color;

void main()
{
   if(use_texture > 0.5f)
   {
      float out_alpha = texture(ui_texture, tex_coords).a;
      out_color = vec4(texture(ui_texture, tex_coords).rgb, out_alpha * alpha);
   }
   else 
   {
      out_color = vec4(color, alpha);
   }
}


