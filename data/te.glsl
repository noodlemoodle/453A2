#version 430

layout(isolines) in;

void main()
{

    float t = gl_TessCoord.x;
    float tquad = t * t;
    float tcubic = tquad * t;

    	float b0 = (-0.5*t + tquad - 0.5 * tcubic);
	float b1 = (1 - 2.5*tquad + 1.5 * tcubic);
	float b2 = (0.5*t + 2 * tquad - 1.5 * tcubic);
	float b3 = (-0.5*tquad + 0.5 * tcubic);

	gl_Position = b0 * gl_in[0].gl_Position + b1 * gl_in[1].gl_Position + b2 * gl_in[2].gl_Position + b3 * gl_in[3].gl_Position;

}
