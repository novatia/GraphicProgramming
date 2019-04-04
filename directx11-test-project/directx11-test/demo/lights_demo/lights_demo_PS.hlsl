struct Material {
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

struct DirectionalLight {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 dirW;
};

struct PointLight {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 posW;
	float range;
	float3 attenuation;
};

struct SpotLight {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 posW;
	float range;
	float3 dirW;
	float spot;
	float3 attenuation;
};

cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 WT;
	float4x4 WVP;
	float4x4 transform;
	Material material;
};

cbuffer PerFrameCB : register(b1)
{
	DirectionalLight d;
	PointLight p;
	SpotLight s;
	float3 eyePosW;
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

void DirectionalLightContribution(Material mat, DirectionalLight light, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular) 
{
	ambient  = float4(0.f, 0.f, 0.f, 0.f);
	diffuse  = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	ambient += mat.ambient * light.ambient;

	float3 toLightW = -light.dirW;
	float Kd = dot( toLightW, normalW );

	[flatten]
	if (Kd > 0.f)
	{
		diffuse += Kd * mat.diffuse*light.diffuse;

		float3 halfVectorW = normalize(toLightW+toEyeW);
		float Ks = pow(max(dot(halfVectorW,normalW),0.f),mat.specular.w);
		specular += Ks * mat.specular*light.specular;
	}
}

void PointLightContribution(Material mat, PointLight light, float3 posW, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	float3 toLightW = ( light.posW - posW );
	float distance = length(toLightW);

	if (distance > light.range)
		return;
	
	toLightW = normalize(toLightW);

	ambient += mat.ambient * light.ambient;

	float Kd = dot(toLightW, normalW);

	[flatten]
	if (Kd > 0.f)
	{
		diffuse += Kd * mat.diffuse * light.diffuse;

		float3 halfVectorW = normalize(toLightW + toEyeW);
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		specular += Ks * mat.specular*light.specular;
	}

	//attenuazione
	float attenuation = 1 / dot(light.attenuation, float3(1, distance, distance*distance));

	diffuse *= attenuation;
	specular *= specular;
}

void SpotLightContribution(Material mat, SpotLight light, float3 posW, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	float3 toLightW = (light.posW - posW);

	float distance = length(toLightW);
	if (distance > light.range)
		return;
	
	toLightW = normalize(toLightW);
	ambient += mat.ambient * light.ambient;

	float Kd = dot(toLightW, normalW);

	[flatten]
	if (Kd > 0.f)
	{
		diffuse += Kd * mat.diffuse * light.diffuse;

		float3 halfVectorW = normalize(toLightW + toEyeW);
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		specular += Ks * mat.specular*light.specular;
	}

	float spot = pow(max(dot(-toLightW, light.dirW), 0.f), light.spot);

	float attenuation = spot / dot(light.attenuation, float3(1, distance, distance*distance));

	ambient *= spot;
	diffuse *= attenuation;
	specular *= attenuation;

}

float4 main( VertexOut pin ) : SV_Target
{
	pin.NORMAL_W  = normalize(pin.NORMAL_W);

	float3 toEyeW = normalize( eyePosW - pin.POSITION_W);

	float4 totalAmbient  = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalDiffuse  = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalSpecular = float4(0.f, 0.f, 0.f, 0.f);

	float4 ambient;
	float4 diffuse;
	float4 specular;

	if (useDLight) {
		DirectionalLightContribution(material, d, pin.NORMAL_W, toEyeW, ambient, diffuse, specular);

		totalAmbient  += ambient;
		totalDiffuse  += diffuse;
		totalSpecular += specular;
	}

	if (usePLight) {
		PointLightContribution(material, p, pin.POSITION_W, pin.NORMAL_W, toEyeW, ambient, diffuse, specular);

		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	if (useSLight) {
		SpotLightContribution(material, s, pin.POSITION_W, pin.NORMAL_W, toEyeW, ambient, diffuse, specular);

		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	
	float4 finalColor = totalAmbient + totalDiffuse + totalSpecular;
	finalColor.a = totalDiffuse.a;

	return finalColor;
	//return float4(d.dirW, 1) ;	
	//return float4(toEyeW, 1);
	//return float4(pin.NORMAL_W,1);
}