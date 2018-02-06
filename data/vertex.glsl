#version 430

layout(location = 0) in vec2 windowCoord;
layout(location = 1) in vec2 texCoord;

out vec2 textureCoords;

uniform float scroll;
uniform vec2 offset;

void main()
{
     vec2 newCoord;

     newCoord.x = offset.x+windowCoord.x*scroll;
     newCoord.y = offset.y+windowCoord.y*scroll;

     gl_Position = vec4(newCoord, 0.0, 1.0);

     textureCoords = texCoord;
}
