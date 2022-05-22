// Input: Position and Color
// Output: VSOut
// SV: system value. SV_Position is like gl_Position

struct VSOut
{
	// note: color has to be the first element in VSOut in this demo
	// Otherwise, the input to pixel shader won't be matched to this color
	float3 color : Color;
	float4 pos : SV_Position;
};

cbuffer CBuf
{
	// HLSL expects column major matrix, but we pass in row major matrix
	// so we can specify that we use row major matrix like below:
	// row_major matrix transform;

	// However, in this case, we use DirectXMath to tranpose the matrix in CPU side,
	// and pass it as column major matrix. row_major is no longer needed. 
	matrix transform;
};

VSOut main(float3 pos : Position, float3 color : Color)
{
	VSOut vso;
	vso.pos = mul(float4(pos, 1.0f), transform);
	vso.color = color;
	return vso;
}