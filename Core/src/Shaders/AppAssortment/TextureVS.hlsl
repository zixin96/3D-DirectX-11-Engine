cbuffer CBuf
{
    matrix modelViewProj;
    matrix modelView;
};

struct VSOut
{
	float2 tex : TexCoord;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float2 tex : TexCoord)
{
	VSOut vso;
    vso.pos = mul(float4(pos, 1.0f), modelViewProj);
	vso.tex = tex;
	return vso;
}
