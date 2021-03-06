cbuffer LightCBuf : register(b0)
{
	float3 lightPos;
	float3 ambient;
	float3 diffuseColor;
	float diffuseIntensity;
	float attConst;
	float attLin;
	float attQuad;
};

cbuffer ObjectCBuf : register(b1)
{
	float specularIntensity;
	float specularPower;
	float padding[2];
};

float4 main(float3 posCamSpace : Position, float3 normalCamSpace : Normal, float3 color : Color) : SV_Target
{
	// fragment to light vector data
    const float3 nHat = normalize(normalCamSpace);
	const float3 vToL = lightPos - posCamSpace;
	const float distToL = length(vToL);
	const float3 dirToL = vToL / distToL;
	// attenuation
	const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// diffuse intensity
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, nHat));
	// reflected light vector
    const float3 w = nHat * dot(vToL, nHat);
	const float3 r = w * 2.0f - vToL;
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
	const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(
		max(0.0f, dot(normalize(-r), normalize(posCamSpace))), specularPower);
	// final color
	return float4(saturate((diffuse + ambient + specular) * color), 1.0f);
}
