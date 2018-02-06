#version 430

in vec2 textureCoords;
out vec4 FragmentColour;

uniform sampler2DRect tex;
uniform int grayScale;
uniform int twoBit;

float quantize(float x) {
     if(x>0&&x<0.25){return 0.25;}
     if(x>=0.25&&x<0.50){return 0.50;}
     if(x>=0.50&&x<0.75){return 0.75;}
     if(x>=0.75&&x<=1.0){return 1.0;}
}

void main(void)
{
    vec4 colour = texture(tex, textureCoords);

    if(grayScale == 1) {
          vec3 gray;
          gray.x = colour.x*0.2989 + colour.y*0.5870 + colour.z*0.1140;
          gray.y = colour.x*0.2989 + colour.y*0.5870 + colour.z*0.1140;
          gray.z = colour.x*0.2989 + colour.y*0.5870 + colour.z*0.1140;
          colour = vec4(gray, 1.0);
     }

     if(twoBit == 1) {
          colour.x = quantize(colour.x);
          colour.y = quantize(colour.y);
          colour.z = quantize(colour.z);
     }

    FragmentColour = colour;
}
