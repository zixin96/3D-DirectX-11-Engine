// This buffer is set per frame for the light (default to slot 0)
cbuffer LightCBuf : register(b0)
{
// we can update lighting parameters every frame

// HSLL expects 16-byte alignment. Thus, in C++ side, we should use alignas or add paddings to ensure proper alignment
float3 lightPosCamSpace;
float3 ambient;
float3 diffuseColor;
float diffuseIntensity;
float attConst;
float attLin;
float attQuad;
};

// This buffer is set per object for each object we want to render (default to slot 1)
cbuffer ObjectCBuf : register(b1)
{
float3 materialColors[6];
// the reason why we need a padding here is because GPU will not pad the the last element in the array,
// it will just use the next float to fill in (b/c a single float don't have alignment requirements)
// If we omit this padding, then specularIntensity is lost 
float padding;
float specularIntensity;
float specularPower;
};

float4 main(float3 posCamSpace : Position, float3 normalCamSpace : Normal, uint tid : SV_PrimitiveID) : SV_Target
{
	// fragment to light vector data
	const float3 vToL = lightPosCamSpace - posCamSpace;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// attenuation
	const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// diffuse intensity
	const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, normalCamSpace));
	// reflected light vector (could use HLSL reflect)
	const float3 w = normalCamSpace * dot(vToL, normalCamSpace);
	const float3 r = w * 2.0f - vToL;
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
	const float3 specular = att
		* (diffuseColor * diffuseIntensity)
		* specularIntensity
		* pow(max(0.0f, dot(normalize(-r), normalize(posCamSpace))), specularPower);

	// final color
	return float4(saturate((diffuse + ambient + specular) * materialColors[(tid / 2) % 6]), 1.0f);
}
