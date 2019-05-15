
{
	UINT mWidth = 2048;
	UINT mHeight= 2048;
		
	D3D11_VIEWPORT mViewport;
	// Viewport that matches the shadow map dimensions.
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width    = static_cast<float>(width);
	mViewport.Height   = static_cast<float>(height);
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;
	
	
	// Use typeless format because the DSV is going to interpret
	// the bits as DXGI_FORMAT_D24_UNORM_S8_UINT, whereas the SRV is going
	// to interpret the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width     = mWidth;
	texDesc.Height    = mHeight;
	texDesc.Format    = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count   = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage          = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags      = 0;
	
	ID3D11Texture2D* depthMap = 0;
	
	HR(device->CreateTexture2D(&texDesc, 0, &depthMap));
	
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	
	ID3D11DepthStencilView* mDepthMapDSV;
	HR(device->CreateDepthStencilView(depthMap, &dsvDesc, &mDepthMapDSV));
	
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	
	ID3D11ShaderResourceView* mDepthMapSRV;
	HR(device->CreateShaderResourceView(depthMap, &srvDesc, &mDepthMapSRV));
	// View saves a reference to the texture so we can release our
	// reference.
	ReleaseCOM(depthMap);
}

ID3D11ShaderResourceView* ShadowMap::DepthMapSRV()
{
	return mDepthMapSRV;
}



void ShadowMap::BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* dc)
{
	dc->RSSetViewports(1, &mViewport);
	// Set null render target because we are only going to draw
	// to depth buffer. Setting a null render target will disable
	// color writes.
	ID3D11RenderTargetView*
	renderTargets[1] = {0};
	dc->OMSetRenderTargets(1, renderTargets, mDepthMapDSV);
	dc->ClearDepthStencilView(mDepthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


Render () {
	// Estimate the scene bounding sphere manually since we know
// how the scene was constructed. The grid is the "widest object"
// with a width of 20 and depth of 30.0f, and centered at the world
// space origin. In general, you need to loop over every world space
// vertex position and compute the bounding sphere.
struct BoundingSphere
{
BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
XMFLOAT3 Center;
float Radius;
};
BoundingSphere mSceneBounds;

mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
mSceneBounds.Radius = sqrtf(10.0f*10.0f + 15.0f*15.0f);
	
}


void ShadowsApp::UpdateScene(float dt)
{
	[...]
	//
	// Animate the lights (and hence shadows).
	//
	mLightRotationAngle += 0.1f*dt;
	XMMATRIX R = XMMatrixRotationY(mLightRotationAngle);

	for(int i = 0; i < 3; ++i)
	{
		XMVECTOR lightDir = XMLoadFloat3(&mOriginalLightDir[i]);
		lightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&mDirLights[i].Direction, lightDir);
	}

	BuildShadowTransform();
}

void ShadowsApp::BuildShadowTransform()
{
	// Only the first "main" light casts a shadow.
	XMVECTOR lightDir = XMLoadFloat3(&mDirLights[0].Direction);
	XMVECTOR lightPos = -2.0f*mSceneBounds.Radius*lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);
	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f);
	XMMATRIX S = V*P*T;
	XMStoreFloat4x4(&mLightView, V);
	XMStoreFloat4x4(&mLightProj, P);
	XMStoreFloat4x4(&mShadowTransform, S);
}

mSmap->BindDsvAndSetNullRenderTarget(md3dImmediateContext);
DrawSceneToShadowMap();
md3dImmediateContext->RSSetState(0);
// Restore the back and depth buffer to the OM stage.
//
ID3D11RenderTargetView* renderTargets[1] = {mRenderTargetView};
md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
md3dImmediateContext->RSSetViewports(1, &mScreenViewport);




/////

cbuffer cbPerFrame
{
	float4x4 gLightWVP;
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexIn
{
float3 PosL : POSITION;
float3 NormalL : NORMAL;
float2 Tex : TEXCOORD;
};

struct VertexOut
{
float4 PosH : SV_POSITION;
float2 Tex : TEXCOORD;
};
VertexOut VS(VertexIn vin)
{
VertexOut vout;
vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
return vout;
}
// This is only used for alpha cut out geometry, so that shadows
// show up correctly. Geometry that does not need to sample a
// texture can use a NULL pixel shader for depth pass.
void PS(VertexOut pin)
{
float4 diffuse = gDiffuseMap.Sample(samLinear, pin.Tex);
// Don′t write transparent pixels to the shadow map.
clip(diffuse.a - 0.15f);
}
RasterizerState Depth
{
// [From MSDN]
// If the depth buffer currently bound to the output-merger stage has a UNORM
// format or no depth buffer is bound the bias value is calculated like this:
//
// Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
//
// where r is the minimum representable
value > 0 in the depth-buffer format
// converted to float32.
// [/End MSDN]
//
// For a 24-bit depth buffer, r = 1 / 2^24.
//
// Example: DepthBias = 100000 ==> Actual DepthBias = 100000/2^24 = .006
// You need to experiment with these values for your scene.
DepthBias = 100000;
DepthBiasClamp = 0.0f;
SlopeScaledDepthBias = 1.0f;
};
technique11 BuildShadowMapTech
{
pass P0
{
SetVertexShader(CompileShader(vs_5_0, VS()));
SetGeometryShader(NULL);
SetPixelShader(NULL);
SetRasterizerState(Depth);
}
}
technique11 BuildShadowMapAlphaClipTech
{
pass P0
{
SetVertexShader(CompileShader(vs_5_0, VS()));
SetGeometryShader(NULL);
SetPixelShader(CompileShader(ps_5_0, PS()));
}
}



#include "LightHelper.fx"
cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;
	float gFogStart;
	float gFogRange;
	float4 gFogColor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	float4x4 gShadowTransform;
	Material gMaterial;
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gShadowMap;
Texture2D gDiffuseMap;
Texture2D gNormalMap;
TextureCube gCubeMap;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerComparisonState samShadow
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	ComparisonFunc = LESS_EQUAL;
};

//HLSL

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;
	float3 TangentL : TANGENT;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	// Transform to world space space.
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = mul(vin.TangentL, (float3x3)gWorld);
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	// Generate projective tex-coords to project shadow map onto scene.
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);
	return vout;
}

//HLSL PIXEL SHADER

float4 PS(VertexOut pin,
uniform int gLightCount,
uniform bool gUseTexure,
uniform bool gAlphaClip,
uniform bool gFogEnabled,
uniform bool gReflectionEnabled) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
	pin.NormalW = normalize(pin.NormalW);
	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;
	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);
	// Normalize.
	toEye /= distToEye;
	// Default to multiplicative identity.
	float4 texColor = float4(1, 1, 1, 1);
	if(gUseTexure)
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample(samLinear, pin.Tex);
		f(gAlphaClip)
		{
			// Discard pixel if texture alpha < 0.1. Note that we
			// do this test as soon as possible so that we can
			// potentially exit the shader early, thereby skipping
			// the rest of the shader code.
			clip(texColor.a - 0.1f);
		}
	}

	//
	// Normal mapping
	//
	float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);
	//
	// Lighting.
	//
	float4 litColor = texColor;
	if(gLightCount > 0)
	{
		
		// Start with a sum of zero.
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
		
		// Only the first light casts a shadow.
		float3 shadow = float3(1.0f, 1.0f, 1.0f);
		shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);
		// Sum the light contribution from each light source.
		[unroll]
		for(int i = 0; i < gLightCount; ++i)
		{
		float4 A, D, S;
		ComputeDirectionalLight(gMaterial, gDirLights[i],
		bumpedNormalW, toEye, A, D, S);
		ambient += A;
		diffuse += shadow[i]*D;
		spec += shadow[i]*S;
		}
		litColor = texColor*(ambient + diffuse) + spec;
		if(gReflectionEnabled)
		{
		float3 incident = -toEye;
		float3 reflectionVector = reflect(incident, bumpedNormalW);
		float4 reflectionColor = gCubeMap.Sample(
		   samLinear, reflectionVector);
		litColor +=gMaterial.Reflect*reflectionColor;
		}
	}

	//
	// Fogging
	//
	if(gFogEnabled)
	{
		float fogLerp = saturate((distToEye - gFogStart) / gFogRange);
		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}
	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;
	return litColor;
}

CalcShadowFactor(SamplerComparisonState shadow_sampler, Texture2D shadowMap,	float4 sposH)
{
	// Complete projection by doing division by w.
	sposH.xyz /= sposH.w;
	// Depth in NDC space.
	float depth = sposH.z;
	// Texel size.
	const float dx = SMAP_DX;
	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
	};
	// 3×3 box filter pattern. Each sample does a 4-tap PCF.
	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += sposH.SampleCmpLevelZero(samShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}
	// Average the samples.
	return percentLit /= 9.0f;
}


void ComputeDirectionalLight(Material mat, DirectionalLight L,
         float3 normal, float3 toEye,
         out float4 ambient,
         out float4 diffuse,
         out float4 spec)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -L.Direction;
	// Add ambient term.
	ambient = mat.Ambient * L.Ambient;
	// Add diffuse and specular term, provided the surface is in
	// the line of site of the light.
	float diffuseFactor = dot(lightVec, normal);
	// Flatten to avoid dynamic branching.
	[flatten]
	if(diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}
}