#include "../../helper/Transform.hlsl"

struct VSOut
{
	float3 viewPos : Position;
	float3 viewNormal : Normal;
	float3 viewTangent : Tangent;
	float3 viewBitangent : Bitangent;
	float2 tc : TexCoord; // name must match what we set in Vertex.h
	float4 pos : SV_Position;
};

VSOut main(float3 inPos : Position, float3 inNormal : Normal, float3 inTangent : Tangent, float3 inBitangent : Bitangent, float2 inTexCoord : TexCoord)
{
	VSOut vso;
	vso.viewPos       = (float3)mul(float4(inPos, 1.0f), modelView);
	vso.viewNormal    = mul(inNormal, (float3x3)modelView); // model matrix to 3x3 since we don't want to translate the normal
	vso.viewTangent   = mul(inTangent, (float3x3)modelView);
	vso.viewBitangent = mul(inBitangent, (float3x3)modelView);
	vso.pos           = mul(float4(inPos, 1.0f), modelViewProj);
	vso.tc            = inTexCoord;
	return vso;
}
