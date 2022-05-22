// Input: float2
// Output: float4
// SV: system value. SV_Position is like gl_Position
float4 main(float2 pos : Position) : SV_Position
{
	return float4(pos.x, pos.y, 0.0f, 1.0f);
}