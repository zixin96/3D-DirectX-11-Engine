// This buffer is set per frame for the light (default to slot 0)
cbuffer LightCBuf : register(b0)
{
// we can update lighting parameters every frame

// HSLL expects 16-byte alignment. Thus, in C++ side, we should use alignas or add paddings to ensure proper alignment
float3 lightPosCamSpace;
float3 ambient;
float3 diffuseColor;
float  diffuseIntensity;
float  attConst;
float  attLin;
float  attQuad;
};

cbuffer ObjectCBuf : register(b1)
{
float specularIntensity;
float specularPower;
// bool: 4 bytes, not 1 byte like C++
bool  normalMapEnabled;
float padding[1];
};

cbuffer TransformCBuf : register(b2)
{
matrix modelViewProj;
matrix modelView;
};

Texture2D diffuseMap;
Texture2D normalMap;

SamplerState splr;

float4 main(float3 posCamSpace : Position, float3 normalCamSpace : Normal, float2 tc : TexCoord) : SV_Target
{
	// sample normal from map if normal mapping enabled
	if (normalMapEnabled)
	{
		const float3 normalSample = normalMap.Sample(splr, tc).xyz;
		// from [0.0, 1.0] to [-1.0, 1.0]
		normalCamSpace.x = normalSample.x * 2.0f - 1.0f;
		// flip the Y direction b/c directX Y points down and the image UV coordinate, Y points up
		normalCamSpace.y = -(normalSample.y * 2.0f - 1.0f);
		normalCamSpace.z = -normalSample.z;
		normalCamSpace   = mul(normalCamSpace, (float3x3)modelView);
	}

	// remember to normalize the normal!
	const float3 nHat = normalize(normalCamSpace);
	// fragment to light vector data
	const float3 vToL    = lightPosCamSpace - posCamSpace;
	const float  distToL = length(vToL);
	const float3 dirToL  = vToL / distToL;
	// attenuation
	const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// diffuse intensity
	const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, nHat));
	// reflected light vector (could use HLSL reflect)
	const float3 w        = nHat * dot(vToL, nHat);
	const float3 r        = w * 2.0f - vToL;
	const float3 specular = att
	                        * (diffuseColor * diffuseIntensity)
	                        * specularIntensity
	                        * pow(max(0.0f, dot(normalize(r), normalize(-posCamSpace))), specularPower);

	// final color
	return float4(saturate((diffuse + ambient) * diffuseMap.Sample(splr, tc).rgb + specular), 1.0f);
}
