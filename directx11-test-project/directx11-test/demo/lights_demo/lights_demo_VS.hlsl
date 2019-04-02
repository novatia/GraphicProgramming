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
	Material material;
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
	float4 NORMAL_W :NORMAL;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.POSITION_HW = mul(float4(vin.POSITION_L,1), WVP);
	vout.NORMAL_W = mul(float4(vin.NORMAL_L, 1), WT );
	
	return vout;
}
