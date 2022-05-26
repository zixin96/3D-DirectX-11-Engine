cbuffer CBuf : register(b0)
{
matrix modelViewProj;
matrix modelView;
};

struct VSOut
{
	float3 posCamSpace : Position;
	float3 normalCamSpace : Normal;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 n : Normal)
{
	VSOut vso;
	vso.posCamSpace = (float3)mul(float4(pos, 1.0f), modelView);
	// model matrix to 3x3 since we don't want to translate the normal
	vso.normalCamSpace = mul(n, (float3x3)modelView);
	vso.pos = mul(float4(pos, 1.0f), modelViewProj);
	return vso;
}
