struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 options; //0 has texture //1 texture tiles //2 displace //3 animated
};


struct VertexIn
{
	float3 posL		: POSITION;
	float3 normalL	: NORMAL;
	float3 tangentL	: TANGENT;
	float2 uv		: TEXCOORD;
};

struct VertexOut
{
	float4 posH		: SV_POSITION;
	float3 posW		: POSITION;
	float3 normalW	: NORMAL;
	float3 tangentW	: TANGENT;
	float2 uv		: TEXCOORD;
	float2 uvAnim	: TEXCOORD1;

};


cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	float4x4 TextCoordMatrix;
	Material material;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posW = mul(float4(vin.posL, 1.0f), W).xyz;
	vout.normalW = mul(vin.normalL, (float3x3)W_inverseTraspose);
	vout.posH = mul(float4(vin.posL, 1.0f), WVP);

	vout.tangentW = mul(vin.tangentL, (float3x3)W_inverseTraspose);

	if ( material.options.x == 3) {
		vout.uvAnim = mul(float4(vin.uv, 0.f, 1.f), TextCoordMatrix).xy;
	}
	else {
		vout.uvAnim = vin.uv;
	}

	vout.uv = vin.uv;

	return vout;
}