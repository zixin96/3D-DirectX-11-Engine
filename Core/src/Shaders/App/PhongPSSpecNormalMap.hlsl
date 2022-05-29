#include "../helper/ShaderOps.hlsl"
#include "../helper/LightVectorData.hlsl"
#include "../helper/PointLight.hlsl"

cbuffer ObjectCBuf
{
bool normalMapEnabled;
bool specularMapEnabled;
// does the specular texture map has alpha value? 
bool   hasGloss;
float  specularPowerConst;
float3 specularColor;
float  specularMapWeight;
};

Texture2D diffuseMap;
Texture2D specularMap;
Texture2D normalMap;

SamplerState splr;

float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal, float3 viewTan : Tangent, float3 viewBitan : Bitangent, float2 tc : Texcoord) : SV_Target
{
	// normalize the mesh normal
	viewNormal = normalize(viewNormal);
	// replace normal with mapped if normal mapping enabled
	if (normalMapEnabled)
	{
        viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, normalMap, splr);
    }
	// fragment to light vector data
	const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
	// specular parameter determination (mapped or uniform)
	float3 specularReflectionColor;
	float  specularPower = specularPowerConst;
	if (specularMapEnabled)
	{
        const float4 specularSample = specularMap.Sample(splr, tc);
		specularReflectionColor     = specularSample.rgb * specularMapWeight;
		if (hasGloss)
		{
			specularPower = pow(2.0f, specularSample.a * 13.0f);
		}
	}
	else
	{
		specularReflectionColor = specularColor;
	}
	// attenuation
	const float att = Attenuate(attConst, attLinear, attQuad, lv.distToL);
	// diffuse light
	const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
	// specular reflected
	const float3 specularReflected = Speculate(
	                                           specularReflectionColor, 1.0f, viewNormal,
	                                           lv.vToL, viewFragPos, att, specularPower
	                                          );
	// final color = attenuate diffuse & ambient by diffuse texture color and add specular reflected
    return float4(saturate((diffuse + ambient) * diffuseMap.Sample(splr, tc).rgb + specularReflected), 1.0f);
}
