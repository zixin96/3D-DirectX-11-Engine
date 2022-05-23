cbuffer CBuf
{
// HLSL expects column major matrix, but we pass in row major matrix
// so we can specify that we use row major matrix like below:
// row_major matrix transform;

// However, in this case, we use DirectXMath to tranpose the matrix in CPU side,
// and pass it as column major matrix. row_major is no longer needed. 
matrix transform;
};

float4 main(float3 pos : Position) : SV_Position
{
	return mul(float4(pos, 1.0f), transform);
}
