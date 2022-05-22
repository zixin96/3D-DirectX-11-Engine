// input: Color from vertex shader 
// output: color represented by float4
// SV_Target: render target
float4 main(float3 color : Color) : SV_Target
{
	return float4(color, 1.0f);
}