struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	float4x4 TexcoordMatrix;
	float4x4 WVP_shadowMap;
	float4x4 WVPT_shadowMap;
	Material material;
};

float4 main(float3 posL:POSITION ):SV_POSITION
{
	return mul(float4(posL,1), WVP_shadowMap);
}