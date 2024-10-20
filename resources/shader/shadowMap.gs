#version 410 core

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16]; // UBO for 16 light space matrices
};
in vec2 TexCoord[];
out vec2 FragTexCoord; // Declare output for texture coordinates

void main()
{
	for (int i = 0; i < 3; ++i)
	{
		gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		FragTexCoord = TexCoord[i];
		EmitVertex();
	}
	EndPrimitive();
}