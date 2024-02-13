// Input vertex data structure
struct VertexInputType
{
	float3 position : POSITION;
	float2 tex : TEXCOORD0;
};

// Output vertex data structure
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

// Basic vertex shader
PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	// Pass through the vertex position and texture coordinates
	output.position = float4(input.position.x, input.position.y, input.position.z, 1);
	output.tex = input.tex;

	return output;
}
