#include "../../helper/Transform.hlsl"

struct VSOut
{
	float3 viewPos : Position;
	float3 viewNormal : Normal;
	float4 pos : SV_Position;
};

VSOut main(float3 inPos : Position, float3 inNormal : Normal)
{
	VSOut vso;
	vso.viewPos    = (float3)mul(float4(inPos, 1.0f), modelView);
	vso.viewNormal = mul(inNormal, (float3x3)modelView);
	vso.pos        = mul(float4(inPos, 1.0f), modelViewProj);
	return vso;
}
