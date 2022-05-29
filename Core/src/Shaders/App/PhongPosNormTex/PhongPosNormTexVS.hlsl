#include "../../helper/Transform.hlsl"

struct VSOut
{
	float3 viewPos : Position;
	float3 viewNormal : Normal;
	float2 tc : TexCoord;
	float4 pos : SV_Position;
};

VSOut main(float3 inPos : Position, float3 inNormal : Normal, float2 inTexCoord : TexCoord)
{
	VSOut vso;
	vso.viewPos    = (float3)mul(float4(inPos, 1.0f), modelView);
	vso.viewNormal = mul(inNormal, (float3x3)modelView); // model matrix to 3x3 since we don't want to translate the normal
	vso.pos        = mul(float4(inPos, 1.0f), modelViewProj);
	vso.tc         = inTexCoord;
	return vso;
}
