cbuffer CBuf
{
matrix modelViewProj;
matrix modelView;
};

struct VSOut
{
    float3 posCamSpace : Position;
    float3 normalCamSpace : Normal;
	float2 tc : Texcoord;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 n : Normal, float2 tc : Texcoord)
{
	VSOut vso;
    vso.posCamSpace = (float3) mul(float4(pos, 1.0f), modelView);
    vso.normalCamSpace = mul(n, (float3x3) modelView);
	vso.pos = mul(float4(pos, 1.0f), modelViewProj);
	vso.tc = tc;
	return vso;
}
