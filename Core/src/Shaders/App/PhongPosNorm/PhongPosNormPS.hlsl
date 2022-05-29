#include "../../helper/PointLight.hlsl"
#include "../../helper/LightVectorData.hlsl"
#include "../../helper/ShaderOps.hlsl"

cbuffer ObjectCBuf : register(b1)
{
float3 materialColor;
float  specularIntensity;
float  specularPower;
};

float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal) : SV_Target
{
	// normalize the mesh normal
	viewNormal = normalize(viewNormal);
	// fragment to light vector data
	const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
	// attenuation
	const float att = Attenuate(attConst, attLinear, attQuad, lv.distToL);
	// diffuse
	const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
	// specular
	const float3 specular = Speculate(
	                                  diffuseColor, diffuseIntensity, viewNormal,
	                                  lv.vToL, viewFragPos, att, specularPower
	                                 );
	// final color
	return float4(saturate((diffuse + ambient) * materialColor.rgb + specular), 1.0f);
}
