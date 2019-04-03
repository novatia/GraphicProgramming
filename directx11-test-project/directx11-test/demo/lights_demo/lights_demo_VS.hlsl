struct Material {
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
	float3 NORMAL_L : NORMAL;
	float3 TANGENT_L : NORMAL1;
	float4 UV_L: TEXCOORD;
};

struct VertexOut
{
	float4 POSITION_HW :SV_POSITION;
	float3 NORMAL_W :NORMAL;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	float4 POSITION_H = float4(vin.POSITION_L, 1);

	float4 POSITION_LT	= mul(TRANSFORM,POSITION_H);
	//vout.POSITION_HW	= mul( POSITION_LT, WVP );
	vout.POSITION_HW = mul(  WVP , POSITION_LT);

	float4 NORMAL_H = float4(vin.NORMAL_L, 1);
	float4 NORMAL_LT	= mul(TRANSFORM, NORMAL_H);
	//float4 NORMAL_LT = float4(vin.NORMAL_L, 1);
	float4 NORMAL_WT = mul( WT, NORMAL_LT);

	vout.NORMAL_W		= float3(NORMAL_WT.x, NORMAL_WT.y, NORMAL_WT.z);

	return vout;
}
