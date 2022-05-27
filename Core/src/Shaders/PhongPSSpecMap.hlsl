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

Texture2D tex;
Texture2D spec;

SamplerState splr;

//static const float specularPowerFactor = 100.0f;

float4 main(float3 posCamSpace : Position, float3 normalCamSpace : Normal, float2 tc : TexCoord) : SV_Target
{
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
	const float4 specularSample = spec.Sample(splr, tc);
	// assume rgb is color intensity, a is power
	const float3 specularReflectionColor = specularSample.rgb;

	// we cannot directly use the alpha channel sampled from the specular map because it ranges from 0.0f to 1.0f, whereas specular power ranges from 1 to 100s
	// so there we map the sampled alpha to a larger range (ask original author of the mesh to obtain the intended mapping)
	// linear scale: 
	// const float  specularPower = specularSample.a * specularPowerFactor;
	// exponential scale: 
    const float specularPower = pow(2.0f, specularSample.a * 13.0f);

	const float3 specular      = att
	                             * (diffuseColor * diffuseIntensity)
	                             * pow(max(0.0f, dot(normalize(r), normalize(-posCamSpace))), specularPower);

	// final color
	return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular * specularReflectionColor), 1.0f);
}
