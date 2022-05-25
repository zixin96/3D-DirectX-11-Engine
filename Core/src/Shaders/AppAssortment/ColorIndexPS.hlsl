cbuffer CBuf
{
	float4 face_colors[8];
};

float4 main( uint tid : SV_PrimitiveID ) : SV_Target
{
	// since our meshes in this demo can have hundreds of triangles,
	// we need to % 8 to ensure we are not accessing out-of-bound colors
	return face_colors[(tid/2) % 8];
}