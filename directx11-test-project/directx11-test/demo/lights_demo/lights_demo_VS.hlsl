struct Material 
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 WT;
	float4x4 WVP;
	float4x4 TRANSFORM;
};

struct VertexIn
{
	float3 POSITION_L : POSITION;
	float3 NORMAL_L   : NORMAL;
	float3 TANGENT_L  : NORMAL1;
	float4 UV_L		  : TEXCOORD;
};

struct VertexOut
{
	float4 POSITION_H : SV_POSITION;
	float3 POSITION_W : POSITION;
	float3 NORMAL_W   : NORMAL;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	float4 POSITION_LH	= float4(vin.POSITION_L, 1);
	float4 POSITION_LT	= mul( TRANSFORM, POSITION_LH);
	vout.POSITION_H		= mul( WVP , POSITION_LT );

	vout.POSITION_W		= mul( W, POSITION_LH ).xyz;

	float4 NORMAL_LH = float4(vin.NORMAL_L, 1);
	vout.NORMAL_W		= mul( WT, NORMAL_LH).xyz;

	return vout;
}