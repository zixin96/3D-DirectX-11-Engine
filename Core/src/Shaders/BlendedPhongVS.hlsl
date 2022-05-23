cbuffer CBuf
{
matrix modelViewProj;
matrix modelView;
};

struct VSOut
{
	float3 posCamSpace : Position;
	float3 normalCamSpace : Normal;
	float3 color : Color;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 n : Normal, float3 color : Color)
{
	VSOut vso;
	vso.posCamSpace = (float3)mul(float4(pos, 1.0f), modelView);
	vso.normalCamSpace = mul(n, (float3x3)modelView);
	vso.pos = mul(float4(pos, 1.0f), modelViewProj);
	vso.color = color;
	return vso;
}
