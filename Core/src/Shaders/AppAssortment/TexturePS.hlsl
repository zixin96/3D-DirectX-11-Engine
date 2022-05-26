// texture bound to slot 0
// Texture2D tex : register(t0);
Texture2D tex;

SamplerState splr;

float4 main(float2 tc : TexCoord) : SV_Target
{
	return tex.Sample(splr, tc);
}
