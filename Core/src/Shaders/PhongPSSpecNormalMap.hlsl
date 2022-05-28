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
bool  normalMapEnabled;
float padding[3];
};

Texture2D diffuseMap : register(t0);
Texture2D specularMap : register(t1);
Texture2D normalMap : register(t2);

SamplerState splr;

float4 main(float3 posCamSpace : Position, float3 normalCamSpace : Normal, float3 tangentCamSpace : Tangent, float3 bitangentCamSpace : Bitangent, float2 tc : TexCoord) : SV_Target
{
	// sample normal from map if normal mapping enabled
	if (normalMapEnabled)
	{
		// build the tranform (rotation) into tangent space
		const float3x3 tanToView = float3x3(
		                                    normalize(tangentCamSpace),
		                                    normalize(bitangentCamSpace),
		                                    normalize(normalCamSpace)
		                                   );
		// unpack the normal from map into tangent space
		const float3 normalSample = normalMap.Sample(splr, tc).xyz;
        normalCamSpace = normalSample * 2.0f - 1.0f;

		// bring normal from tanspace into view space
		normalCamSpace = mul(normalCamSpace, tanToView);
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
	const float3 w = nHat * dot(vToL, nHat);
	const float3 r = w * 2.0f - vToL;
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
    const float4 specularSample = specularMap.Sample(splr, tc);
	// assume rgb is color intensity, a is power
	const float3 specularReflectionColor = specularSample.rgb;

	// we cannot directly use the alpha channel sampled from the specular map because it ranges from 0.0f to 1.0f, whereas specular power ranges from 1 to 100s
	// so there we map the sampled alpha to a larger range (ask original author of the mesh to obtain the intended mapping)
	// linear scale: 
	// const float  specularPower = specularSample.a * specularPowerFactor;
	// exponential scale: 
	const float specularPower = pow(2.0f, specularSample.a * 13.0f);

	const float3 specular = att
	                        * (diffuseColor * diffuseIntensity)
	                        * pow(max(0.0f, dot(normalize(r), normalize(-posCamSpace))), specularPower);

	// final color
    return float4(saturate((diffuse + ambient) * diffuseMap.Sample(splr, tc).rgb + specular * specularReflectionColor), 1.0f);
}
