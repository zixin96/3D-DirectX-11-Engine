cbuffer CBuf
{
	float4 face_colors[6];
};

// what is tid here?
// tid is a SV_PrimitiveID holds a unique ID that identifies which triangle the pixel shader is currently working with

float4 main(uint tid : SV_PrimitiveID) : SV_Target
{
	// Every two triangles share the same face color: 
	// triangle 0 => face_colors[0]
	// triangle 1 => face_colors[0]

	// triangle 2 => face_colors[1]
	// triangle 3 => face_colors[1]

	// triangle 4 => face_colors[2]
	// triangle 5 => face_colors[2]

	// ...

	return face_colors[tid / 2];
}