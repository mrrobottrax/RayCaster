Texture2D shaderTexture;
SamplerState SampleType;

// Input pixel data structure (should match the output structure of the vertex shader)
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

// Basic pixel shader
float4 main(PixelInputType input) : SV_TARGET
{
	return shaderTexture.Sample(SampleType, input.tex);
}