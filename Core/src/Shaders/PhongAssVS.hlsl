cbuffer CBuf : register(b0)
{
matrix modelViewProj;
matrix modelView;
};

struct VSOut
{
	float3 posCamSpace : Position;
	float3 normalCamSpace : Normal;
	float2 tc : TexCoord; // name must match what we set in Vertex.h
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 n : Normal, float2 tc : TexCoord)
{
	VSOut vso;
	vso.posCamSpace    = (float3)mul(float4(pos, 1.0f), modelView);
	vso.normalCamSpace = mul(n, (float3x3)modelView); // model matrix to 3x3 since we don't want to translate the normal
	vso.pos            = mul(float4(pos, 1.0f), modelViewProj);
	vso.tc             = tc;
	return vso;
}
