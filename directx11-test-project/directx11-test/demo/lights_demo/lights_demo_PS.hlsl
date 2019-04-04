struct Material {
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

struct DirectionalLight {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 dirW;
};

struct PointLight {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 posW;
	float4 attenuation;
	float4 range;
};

struct SpotLight {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 dirW;
	float4 posW;
	float4 attenuation;
	float4 range;
	float spot;
};

cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 WT;
	float4x4 WVP;
	Material material;
};

cbuffer PerFrameCB : register(b1)
{
	float4 eyePosW;
	DirectionalLight d;
	PointLight p;
	SpotLight s;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useDLight;
	bool usePLight;
	bool useSLight;
};

struct VertexOut
{
	float4 POSITION_HW	: SV_POSITION;
	float3 POSITION_W	: POSITION;
	float3 NORMAL_W		: NORMAL;
};

void DirectionalLightContribution(Material mat, DirectionalLight light, float3 normalW, float4 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular) 
{
	ambient  = float4(0.f, 0.f, 0.f, 0.f);
	diffuse  = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	ambient += mat.ambient * light.ambient;

	float3 toLightW = -light.dirW.xyz;
	float Kd = dot( toLightW, normalW );

	[flatten]
	if (Kd > 0.f)
	{
		diffuse += Kd * mat.diffuse*light.diffuse;

		float3 halfVectorW = normalize( toLightW + toEyeW.xyz);
		float Ks = pow(max(dot(halfVectorW,normalW),0.f),mat.specular.w);
		specular += Ks * mat.specular*light.specular;
	}
}

void PointLightContribution(Material mat, PointLight light, float3 posW, float3 normalW, float4 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	float3 toLightW = ( light.posW.xyz - posW );
	float distance = length(toLightW);

	if (distance > light.range.x)
		return;
	
	toLightW = normalize(toLightW);

	ambient += mat.ambient * light.ambient;

	float Kd = dot(toLightW, normalW);

	[flatten]
	if (Kd > 0.f)
	{
		diffuse += Kd * mat.diffuse * light.diffuse;

		float3 halfVectorW = normalize(toLightW + toEyeW.xyz);
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		specular += Ks * mat.specular*light.specular;
	}

	//attenuazione
	float attenuation = 1 / dot(light.attenuation.xyz, float3(1, distance, distance*distance));

	diffuse *= attenuation;
	specular *= specular;
}

void SpotLightContribution(Material mat, SpotLight light, float3 posW, float3 normalW, float4 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	float3 toLightW = (light.posW.xyz - posW);

	float distance = length(toLightW);
	if (distance > light.range.x)
		return;
	
	toLightW = normalize(toLightW);
	ambient += mat.ambient * light.ambient;

	float Kd = dot(toLightW, normalW);

	[flatten]
	if (Kd > 0.f)
	{
		diffuse += Kd * mat.diffuse * light.diffuse;

		float3 halfVectorW = normalize(toLightW + toEyeW.xyz);
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		specular += Ks * mat.specular*light.specular;
	}

	float spot = pow(max(dot(-toLightW, light.dirW.xyz), 0.f), light.spot);

	float attenuation = spot / dot(light.attenuation.xyz, float3(1, distance, distance*distance));

	ambient *= spot;
	diffuse *= attenuation;
	specular *= attenuation;

}

float4 main( VertexOut pin ) : SV_Target
{
	pin.NORMAL_W  = normalize(pin.NORMAL_W);

	float4 toEyeW = normalize( eyePosW - float4(pin.POSITION_W,1));

	float4 totalAmbient  = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalDiffuse  = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalSpecular = float4(0.f, 0.f, 0.f, 0.f);

	float4 ambient;
	float4 diffuse;
	float4 specular;

	if ( useDLight ) {
		DirectionalLightContribution(material, d, pin.NORMAL_W, toEyeW, ambient, diffuse, specular);

		totalAmbient  += ambient;
		totalDiffuse  += diffuse;
		totalSpecular += specular;
	}

	if ( usePLight ) {
		PointLightContribution(material, p, pin.POSITION_W, pin.NORMAL_W, toEyeW, ambient, diffuse, specular);

		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	if ( useSLight ) {
		SpotLightContribution(material, s, pin.POSITION_W, pin.NORMAL_W, toEyeW, ambient, diffuse, specular);

		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	
	float4 finalColor = totalAmbient + totalDiffuse + totalSpecular;
	finalColor.a = totalDiffuse.a;

	return finalColor;
//	return p.diffuse;	
	//return float4(toEyeW, 1);
	//return float4(pin.NORMAL_W,1);
	//return material.diffuse;
}