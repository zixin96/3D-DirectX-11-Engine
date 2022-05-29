#include "../../helper/ShaderOps.hlsl"
#include "../../helper/LightVectorData.hlsl"
#include "../../helper/PointLight.hlsl"

cbuffer ObjectCBuf : register(b1)
{
float specularIntensity;
float specularPower;
bool  normalMapEnabled;
float padding[1];
};

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t2);

SamplerState splr;

float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal, float3 viewTan : Tangent, float3 viewBitan : Bitangent, float2 tc : Texcoord) : SV_Target
{
	// normalize the mesh normal
	viewNormal = normalize(viewNormal);
	// replace normal with mapped if normal mapping enabled
	if (normalMapEnabled)
	{
		viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, normalMap, splr);
	}
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
	return float4(saturate((diffuse + ambient) * diffuseMap.Sample(splr, tc).rgb + specular), 1.0f);
}
