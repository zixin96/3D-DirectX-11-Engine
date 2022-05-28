cbuffer LightCBuf : register(b0)
{
float3 lightPosCamSpace;
float3 ambient;
float3 diffuseColor;
float  diffuseIntensity;
float  attConst;
float  attLin;
float  attQuad;
};

cbuffer ObjectCBuf
{
bool normalMapEnabled;
bool specularMapEnabled;
// does the specular texture map has alpha value? 
bool   hasGloss;
float  specularPowerConst;
float3 specularColor;
float  specularMapWeight;
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
		normalCamSpace            = normalSample * 2.0f - 1.0f;

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
	float3 specularReflectionColor;
	float  specularPower = specularPowerConst;
	if (specularMapEnabled)
	{
		const float4 specularSample = specularMap.Sample(splr, tc);
		// assume rgb is color intensity, a (if exists) is specular power
		specularReflectionColor = specularSample.rgb * specularMapWeight;
		if (hasGloss)
		{
			// we cannot directly use the alpha channel sampled from the specular map because it ranges from 0.0f to 1.0f, whereas specular power ranges from 1 to 100s
			// so there we map the sampled alpha to a larger range (ask original author of the mesh to obtain the intended mapping)
			// Here, we do exponential mapping:
			specularPower = pow(2.0f, specularSample.a * 13.0f);
		}
	}
	else
	{
		specularReflectionColor = specularColor;
	}
	const float3 specular = att
	                        * (diffuseColor * diffuseIntensity)
	                        * pow(max(0.0f, dot(normalize(r), normalize(-posCamSpace))), specularPower);

	// final color
	return float4(saturate((diffuse + ambient) * diffuseMap.Sample(splr, tc).rgb + specular * specularReflectionColor), 1.0f);
}
