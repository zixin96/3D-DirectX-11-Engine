#include "../../helper/ShaderOps.hlsl"
#include "../../helper/LightVectorData.hlsl"
#include "../../helper/PointLight.hlsl"

cbuffer ObjectCBuf
{
float specularIntensity;
float specularPower;
float padding[2];
};

Texture2D    diffuseMap;
SamplerState splr;

float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal, float2 tc : TexCoord) : SV_Target
{
	// renormalize interpolated normal
	viewNormal = normalize(viewNormal);
	// fragment to light vector data
	const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
	// attenuation
	const float att = Attenuate(attConst, attLinear, attQuad, lv.distToL);
	// diffuse
	const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
	// specular
	const float3 specular = Speculate(diffuseColor, diffuseIntensity, viewNormal, lv.vToL, viewFragPos, att, specularPower);
	// final color
	return float4(saturate((diffuse + ambient) * diffuseMap.Sample(splr, tc).rgb + specular), 1.0f);
}
