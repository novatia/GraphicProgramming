#include "stdafx.h"
#include "lights_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
#include "mesh/mesh_generator.h"

#define VS_PEROBJECT_CONSTANT_BUFFER_REGISTER 0

#define PS_PEROBJECT_CONSTANT_BUFFER_REGISTER 0
#define PS_PERFRAME_CONSTANT_BUFFER_REGISTER 1
#define PS_RARELY_CONSTANT_BUFFER_REGISTER 2

using namespace DirectX;
using namespace xtest;

using xtest::demo::LightDemoApp;
using Microsoft::WRL::ComPtr;

LightDemoApp::LightDemoApp(HINSTANCE instance,
	const application::WindowSettings& windowSettings,
	const application::DirectxSettings& directxSettings,
	uint32 fps /*=60*/)
	: application::DirectxApp(instance, windowSettings, directxSettings, fps)
	, m_vertexBuffer(nullptr)
	, m_indexBuffer(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_inputLayout(nullptr)
	, m_camera(math::ToRadians(90.f), math::ToRadians(30.f), 10.f, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { math::ToRadians(5.f), math::ToRadians(175.f) }, { 3.f, 25.f })
{
}


LightDemoApp::~LightDemoApp()
{}

void LightDemoApp::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitLightsAndMaterials();

	InitMatrices();
	InitShaders();
	InitBuffers();
	InitMeshes();

	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F, input::Key::S, input::Key::D, input::Key::P});

	m_d3dAnnotation->EndEvent();
}

void LightDemoApp::InitMeshes() 
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	PerObjectCB* constantBufferData;

	D3D11_BUFFER_DESC psPerObjConstantBufferDesc;
	psPerObjConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	psPerObjConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
	psPerObjConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psPerObjConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psPerObjConstantBufferDesc.MiscFlags = 0;
	psPerObjConstantBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth =0;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	
	XMMATRIX WVP;
	XMMATRIX WT; 


	// * * * * * * * * * * * *  * * * * * * * * * * * * * * * * *  * * * * * SPHERE START
	//MESH VERTEX AND TRANSFORM PARAMS
	m_sphere.mesh		= mesh::GenerateSphere(2.f, 40, 40);
	m_sphere.position	= XMFLOAT3(3.0f, 2.0f, 2.0f);
	m_sphere.scale		= XMFLOAT3(1.0f, 0.5f, 1.0f);
	m_sphere.rotation	= XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_sphere.material   = metal;
	
	//CREATE VERTEX AND INDEX BUFFERS
	vertexBufferDesc.ByteWidth	= UINT(sizeof(mesh::MeshData::Vertex))* m_sphere.mesh.vertices.size();
	initData.pSysMem			= &m_sphere.mesh.vertices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &initData, &m_sphere.d3dVertexBuffer));

	indexBufferDesc.ByteWidth	= sizeof(uint32) * m_sphere.mesh.indices.size();
	initData.pSysMem = &m_sphere.mesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &initData, &m_sphere.d3dIndexBuffer));
	
	//CREATE PS BUFFER
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&psPerObjConstantBufferDesc, nullptr, &m_sphere.d3dPSPerObjConstantBuffer));

	//CLEAR BUFFER AND BIND TO PS
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	XTEST_D3D_CHECK(m_d3dContext->Map(m_sphere.d3dPSPerObjConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	
	//update the data
	constantBufferData = (PerObjectCB*)mappedResource.pData;
	//XMStoreFloat4x4(&constantBufferData->WVP, XMMatrixTranspose(WVP));
	//XMStoreFloat4x4(&constantBufferData->WT, XMMatrixTranspose(WT));
	//XMStoreFloat4x4(&constantBufferData->W, XMMatrixTranspose(W));
	//XMStoreFloat4x4(&constantBufferData->transform, XMMatrixTranspose(m_sphere.GetTransform()));

	WVP = GetWVPXMMATRIX(m_sphere.GetTransform());
	WT = GetWTXMMATRIX(m_sphere.GetTransform());

	XMStoreFloat4x4(&constantBufferData->WVP, WVP);
	XMStoreFloat4x4(&constantBufferData->WT, WT);
	XMStoreFloat4x4(&constantBufferData->W, m_sphere.GetTransform());

	XMStoreFloat4(&constantBufferData->material.ambient, XMVectorSet(m_sphere.material.ambient.x, m_sphere.material.ambient.y, m_sphere.material.ambient.z, m_sphere.material.ambient.w));
	XMStoreFloat4(&constantBufferData->material.diffuse, XMVectorSet(m_sphere.material.diffuse.x, m_sphere.material.diffuse.y, m_sphere.material.diffuse.z, m_sphere.material.diffuse.w));
	XMStoreFloat4(&constantBufferData->material.specular, XMVectorSet(m_sphere.material.specular.x, m_sphere.material.specular.y, m_sphere.material.specular.z, m_sphere.material.specular.w));
	m_d3dContext->Unmap(m_sphere.d3dPSPerObjConstantBuffer.Get(), 0);



	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * PLANE START

	//MESH VERTEX AND TRANSFORM PARAMS
	m_plane.mesh		= mesh::GeneratePlane(30.f, 30.f, 30, 30);
	m_plane.position	= XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_plane.scale		= XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_plane.rotation	= XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_plane.material	= concrete;
	
	//CREATE VERTEX AND INDEX BUFFERS
	vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex))* m_plane.mesh.vertices.size();
	initData.pSysMem = &m_plane.mesh.vertices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &initData, &m_plane.d3dVertexBuffer));

	indexBufferDesc.ByteWidth = sizeof(uint32) * m_plane.mesh.indices.size();;
	initData.pSysMem = &m_plane.mesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &initData, &m_plane.d3dIndexBuffer));


	//CREATE PS CONSTANT BUFFER
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&psPerObjConstantBufferDesc, nullptr, &m_plane.d3dPSPerObjConstantBuffer));

	//CLEAR BUFFER AND BIND TO PS

	//update the data
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	XTEST_D3D_CHECK(m_d3dContext->Map(m_plane.d3dPSPerObjConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	constantBufferData = (PerObjectCB*)mappedResource.pData;
//	XMStoreFloat4x4(&constantBufferData->WVP, XMMatrixTranspose(WVP));
//	XMStoreFloat4x4(&constantBufferData->WT, XMMatrixTranspose(WT));
//	XMStoreFloat4x4(&constantBufferData->W, XMMatrixTranspose(W));
//	XMStoreFloat4x4(&constantBufferData->transform, XMMatrixTranspose(m_sphere.GetTransform()));

	WVP = GetWVPXMMATRIX (m_plane.GetTransform());
	WT  = GetWTXMMATRIX  (m_plane.GetTransform());

	XMStoreFloat4x4(&constantBufferData->WVP, (WVP));
	XMStoreFloat4x4(&constantBufferData->WT,  (WT));
	XMStoreFloat4x4(&constantBufferData->W,   (m_plane.GetTransform()));

	XMStoreFloat4(&constantBufferData->material.ambient,  XMVectorSet(m_plane.material.ambient.x,  m_plane.material.ambient.y,  m_plane.material.ambient.z,  m_plane.material.ambient.w));
	XMStoreFloat4(&constantBufferData->material.diffuse,  XMVectorSet(m_plane.material.diffuse.x,  m_plane.material.diffuse.y,  m_plane.material.diffuse.z,  m_plane.material.diffuse.w));
	XMStoreFloat4(&constantBufferData->material.specular, XMVectorSet(m_plane.material.specular.x, m_plane.material.specular.y, m_plane.material.specular.z, m_plane.material.specular.w));
	m_d3dContext->Unmap(m_plane.d3dPSPerObjConstantBuffer.Get(), 0);


	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * BOX START

	//
	m_cube.mesh			= mesh::GenerateBox(1.f, 1.f, 1.f);
	m_cube.position		= XMFLOAT3(-3.0f, 1.0f, -3.0f);
	m_cube.scale		= XMFLOAT3(2.0f, 2.0f, 2.0f);
	m_cube.rotation		= XMFLOAT3(45.0f, 45.0f, 45.0f);
	m_cube.material		= metal;


	//
	vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex))* m_cube.mesh.vertices.size();
	initData.pSysMem = &m_cube.mesh.vertices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &initData, &m_cube.d3dVertexBuffer));

	indexBufferDesc.ByteWidth = sizeof(uint32) * m_cube.mesh.indices.size();;
	initData.pSysMem = &m_cube.mesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&indexBufferDesc, &initData, &m_cube.d3dIndexBuffer));
	
	//CREATE PS CONSTANT BUFFER
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&psPerObjConstantBufferDesc, nullptr, &m_cube.d3dPSPerObjConstantBuffer));
	//CLEAR BUFFER AND BIND TO PS
	//update the data
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	XTEST_D3D_CHECK(m_d3dContext->Map(m_cube.d3dPSPerObjConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	constantBufferData = (PerObjectCB*)mappedResource.pData;
	//XMStoreFloat4x4(&constantBufferData->WVP, XMMatrixTranspose(WVP));
	//XMStoreFloat4x4(&constantBufferData->WT, XMMatrixTranspose(WT));
	//XMStoreFloat4x4(&constantBufferData->W, XMMatrixTranspose(W));
	//XMStoreFloat4x4(&constantBufferData->transform, XMMatrixTranspose(m_sphere.GetTransform()));
	WVP = GetWVPXMMATRIX(m_cube.GetTransform());
	WT = GetWTXMMATRIX(m_cube.GetTransform());

	XMStoreFloat4x4(&constantBufferData->WVP, GetWVPXMMATRIX (m_cube.GetTransform()));
	XMStoreFloat4x4(&constantBufferData->WT, GetWTXMMATRIX(m_cube.GetTransform()));
	XMStoreFloat4x4(&constantBufferData->W, m_cube.GetTransform());

	XMStoreFloat4(&constantBufferData->material.ambient, XMVectorSet(m_cube.material.ambient.x, m_cube.material.ambient.y, m_cube.material.ambient.z, m_cube.material.ambient.w));
	XMStoreFloat4(&constantBufferData->material.diffuse, XMVectorSet(m_cube.material.diffuse.x, m_cube.material.diffuse.y, m_cube.material.diffuse.z, m_cube.material.diffuse.w));
	XMStoreFloat4(&constantBufferData->material.specular, XMVectorSet(m_cube.material.specular.x, m_cube.material.specular.y, m_cube.material.specular.z, m_cube.material.specular.w));
	m_d3dContext->Unmap(m_cube.d3dPSPerObjConstantBuffer.Get(), 0);
	//BOX END
}

void LightDemoApp::InitLightsAndMaterials() {
	//RED
	d1.ambient	= XMFLOAT4(10.0f / 255.0f, 10.0f / 255.0f, 10.0f / 255.0f, 0.0f / 255.0f);
	d1.diffuse	= XMFLOAT4(20.f / 255.0f, 20.f / 255.0f, 20.f / 255.0f, 0.0f / 255.0f);
	d1.specular = XMFLOAT4(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f, 0.0f / 255.0f);

	XMVECTOR dirW = XMLoadFloat4( new XMFLOAT4(-1, -1, -1, 0) );
	XMStoreFloat3( &d1.dirW, dirW);

	//GREEN
	s1.ambient		= XMFLOAT4(0.0f / 255.0f, 55.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f);
	s1.attenuation	= XMFLOAT3(0.5f,0.5f,0.5f);
	s1.diffuse		= XMFLOAT4(255.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f);
	s1.range		= 15.0f;
	s1.specular		= XMFLOAT4(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f);
	s1.spot			= 20.0f;
	//s1.posW			= XMFLOAT3(-10, 10, -10);

	XMVECTOR sdirW = XMLoadFloat4(new XMFLOAT4(-1, -1, -1, 0));
	XMStoreFloat3(&s1.dirW, sdirW);

	XMVECTOR sposW = XMLoadFloat4(new XMFLOAT4(-10, 10, -10, 0));
	XMStoreFloat3(&s1.posW, sposW);

	//BLUE
	p1.ambient = XMFLOAT4(0.0f / 255.0f, 0.0f / 255.0f, 55.0f / 255.0f, 255.0f / 255.0f);
	p1.attenuation = XMFLOAT3(0.5f, 0.5f, 0.5f);
	p1.diffuse = XMFLOAT4(0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
	//p1.posW = XMFLOAT3(10, 10, 10);
	p1.range = 15.0f;
	p1.specular = XMFLOAT4(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
	
	XMVECTOR pposW = XMLoadFloat4( new XMFLOAT4(10, 10, 10,0) );
	XMStoreFloat3(&p1.posW, pposW);


	metal.ambient  = XMFLOAT4(98.0f / 255.0f, 75.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f);
	metal.diffuse  = XMFLOAT4(255.0f / 255.0f, 255.0f / 255.0f, 30.0f / 255.0f, 255.0f / 255.0f);
	metal.specular = XMFLOAT4(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 255.0f / 255.0f);
	
	concrete.ambient = XMFLOAT4(13.0f/255.0f, 13.0f / 255.0f, 13.0f / 255.0f, 255.0f / 255.0f);
	concrete.diffuse = XMFLOAT4(127.0f / 255.0f, 127.0f / 255.0f, 127.0f / 255.0f, 255.0f / 255.0f);
	concrete.specular = XMFLOAT4(10.0f / 255.0f, 10.0f / 255.0f, 10.0f / 255.0f, 128.0f / 255.0f);
}

void LightDemoApp::InitMatrices()
{
	// world matrix
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());

	// view matrix
	XMStoreFloat4x4(&m_viewMatrix, m_camera.GetViewMatrix());

	// projection matrix
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
		XMStoreFloat4x4(&m_projectionMatrix, P);
	}
}

void LightDemoApp::InitShaders()
{
	// read pre-compiled shaders' bytecode
	std::future<file::BinaryFile> psByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\lights_demo_PS.cso"));
	std::future<file::BinaryFile> vsByteCodeFuture = file::ReadBinaryFile(std::wstring(GetRootDir()).append(L"\\lights_demo_VS.cso"));

	// future.get() can be called only once
	file::BinaryFile vsByteCode = vsByteCodeFuture.get();
	file::BinaryFile psByteCode = psByteCodeFuture.get();
	XTEST_D3D_CHECK(m_d3dDevice->CreateVertexShader(vsByteCode.Data(), vsByteCode.ByteSize(), nullptr, &m_vertexShader));
	XTEST_D3D_CHECK(m_d3dDevice->CreatePixelShader(psByteCode.Data(), psByteCode.ByteSize(), nullptr, &m_pixelShader));

	// create the input layout, it must match the Vertex Shader HLSL input format:
	/*	struct VertexIn
	{
		float3 POSITION_L : POSITION;
		float3 NORMAL : NORMAL;
		float3 TANGENT_U : NORMAL1;
		float2 UV : TEXCOORD;
	};
	*/
	int normal_offset = offsetof(VertexIn, normal);
	int tangentU_offset = offsetof(VertexIn, tangentU);
	int uv_offset = offsetof(VertexIn, uv);

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, normal_offset, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   1, DXGI_FORMAT_R32G32B32_FLOAT, 0, tangentU_offset, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, uv_offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	const int vs_size = vsByteCode.ByteSize();
	char* byteCode = new char[vs_size];
	
	memcpy(byteCode, vsByteCode.Data(), vs_size);
	XTEST_D3D_CHECK( m_d3dDevice->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), byteCode, vs_size, &m_inputLayout) );
}

void LightDemoApp::InitBuffers()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Vertex Shader "PerObjectCB" constant buffer:
	D3D11_BUFFER_DESC vsConstantBufferDesc;
	vsConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	vsConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
	vsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vsConstantBufferDesc.MiscFlags = 0;
	vsConstantBufferDesc.StructureByteStride = 0;

	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &m_vsConstantBuffer));

	// Pixel Shader "PerFrameCB" constant buffer:
	D3D11_BUFFER_DESC psPerFrameConstantBufferDesc;
	psPerFrameConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	psPerFrameConstantBufferDesc.ByteWidth = sizeof(PerFrameCB);
	psPerFrameConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psPerFrameConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psPerFrameConstantBufferDesc.MiscFlags = 0;
	psPerFrameConstantBufferDesc.StructureByteStride = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&psPerFrameConstantBufferDesc, nullptr, &m_psPerFrameConstantBuffer));

	// Pixel Shader "RarelyChangedCB" constant buffer:
	D3D11_BUFFER_DESC psRarelyConstantBufferDesc;
	psRarelyConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	psRarelyConstantBufferDesc.ByteWidth = sizeof(RarelyChangedCB);
	psRarelyConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psRarelyConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psRarelyConstantBufferDesc.MiscFlags = 0;
	psRarelyConstantBufferDesc.StructureByteStride = 0;
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&psRarelyConstantBufferDesc, nullptr, &m_psRarelyConstantBuffer));

	{
		//PS UPDATE CONSTANT BUFFER DATA RARELY CHANGING
		D3D11_MAPPED_SUBRESOURCE psRarelyResource;
		ZeroMemory(&psRarelyResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		XTEST_D3D_CHECK(m_d3dContext->Map(m_psRarelyConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &psRarelyResource));
		RarelyChangedCB* psRarelyConstantBufferData = (RarelyChangedCB*)psRarelyResource.pData;
		XMStoreFloat4(&psRarelyConstantBufferData->useDPSLight, XMVectorSet(directionalIsOn, pointIsOn, spotIsOn, 0));
		m_d3dContext->Unmap(m_psRarelyConstantBuffer.Get(), 0);
	}

	m_d3dContext->PSSetConstantBuffers(PS_RARELY_CONSTANT_BUFFER_REGISTER, 1, m_psRarelyConstantBuffer.GetAddressOf());
}

void LightDemoApp::InitRasterizerState()
{
	// rasterizer state
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;

	m_d3dDevice->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);
}

void LightDemoApp::OnResized()
{
	application::DirectxApp::OnResized();

	//update the projection matrix with the new aspect ratio
	XMMATRIX P = XMMatrixPerspectiveFovLH(math::ToRadians(45.f), AspectRatio(), 1.f, 1000.f);
	XMStoreFloat4x4(&m_projectionMatrix, P);
}

void LightDemoApp::OnWheelScroll(input::ScrollStatus scroll)
{
	// zoom in or out when the scroll wheel is used
	if (service::Locator::GetMouse()->IsInClientArea())
	{
		m_camera.IncreaseRadiusBy(scroll.isScrollingUp ? -0.5f : 0.5f);
	}
}

void xtest::demo::LightDemoApp::OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos)
{
	XTEST_UNUSED_VAR(currentPos);

	input::Mouse* mouse = service::Locator::GetMouse();

	// rotate the camera position around the cube when the left button is pressed
	if (mouse->GetButtonStatus(input::MouseButton::left_button).isDown && mouse->IsInClientArea())
	{
		m_camera.RotateBy(math::ToRadians(movement.y * -0.25f), math::ToRadians(movement.x * 0.25f));
	}

	// pan the camera position when the right button is pressed
	if (mouse->GetButtonStatus(input::MouseButton::right_button).isDown && mouse->IsInClientArea())
	{
		XMFLOAT3 cameraX = m_camera.GetXAxis();
		XMFLOAT3 cameraY = m_camera.GetYAxis();

		// we should calculate the right amount of pan in screen space but for now this is good enough
		XMVECTOR xPanTranslation = XMVectorScale(XMLoadFloat3(&cameraX), float(-movement.x) * 0.01f);
		XMVECTOR yPanTranslation = XMVectorScale(XMLoadFloat3(&cameraY), float(movement.y) * 0.01f);

		XMFLOAT3 panTranslation;
		XMStoreFloat3(&panTranslation, XMVectorAdd(xPanTranslation, yPanTranslation));
		m_camera.TranslatePivotBy(panTranslation);
	}

}

void LightDemoApp::OnKeyStatusChange(input::Key key, const input::KeyStatus& status)
{
	XTEST_ASSERT(key == input::Key::F || key == input::Key::S || key == input::Key::D || key == input::Key::P); // the only key registered for this listener

	// re-frame the cube when F key is pressed
	if (status.isDown && key == input::Key::F)
	{
		m_camera.SetPivot({ 0.f, 0.f, 0.f });
		return;
	}

	if (status.isDown && key == input::Key::S) {
		spotIsOn = !spotIsOn;
	}

	if (status.isDown && key == input::Key::D) {
		directionalIsOn = !directionalIsOn;
	}


	if (status.isDown && key == input::Key::P) {
		pointIsOn = !pointIsOn;
	}

	if (status.isDown) {

		{
			//PS UPDATE CONSTANT BUFFER DATA RARELY CHANGING
			D3D11_MAPPED_SUBRESOURCE psRarelyResource;
			ZeroMemory(&psRarelyResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
			XTEST_D3D_CHECK(m_d3dContext->Map(m_psRarelyConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &psRarelyResource));
			RarelyChangedCB* psRarelyConstantBufferData = (RarelyChangedCB*)psRarelyResource.pData;
			XMStoreFloat4(&psRarelyConstantBufferData->useDPSLight, XMVectorSet(directionalIsOn, pointIsOn, spotIsOn, 0));
			m_d3dContext->Unmap(m_psRarelyConstantBuffer.Get(), 0);
		}

		m_d3dContext->PSSetConstantBuffers(PS_RARELY_CONSTANT_BUFFER_REGISTER, 1, m_psRarelyConstantBuffer.GetAddressOf());
	}
}

XMMATRIX LightDemoApp::GetWTXMMATRIX( XMMATRIX W )
{
	W.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR det = XMMatrixDeterminant(W);

	return XMMatrixTranspose(XMMatrixInverse(&det,W));
}

XMMATRIX LightDemoApp::GetWVPXMMATRIX( XMMATRIX W )
{
	XMStoreFloat4x4(&m_worldMatrix, W);
	// create the model-view-projection matrix
	XMMATRIX V = m_camera.GetViewMatrix();
	XMStoreFloat4x4(&m_viewMatrix, V);

	// create projection matrix
	XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);
	XMMATRIX WVP = W * V * P;

	return WVP;
}

float angle = 0;
float pos = 0;
float sign = 1; 

void LightDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);

	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");
	
	//PS UPDATE CONSTANT BUFFER DATA PerFrame
	//LIGHTS
	{
		angle += deltaSeconds;

		float X = 50 * cos(angle);
		float Y = -20;
		float Z = 50 * sin(angle);

		XMVECTOR dirW = XMLoadFloat4(new XMFLOAT4(X, Y, Z, 0));
		XMStoreFloat3( &d1.dirW, dirW);

		if (pos > 2)
			sign = -1;
		
		if (pos < 0)
			sign = 1;

		pos += sign * deltaSeconds;

		float Xp = 10 * pos;
		float Zp = 10 * pos;

		XMVECTOR posW = XMLoadFloat4(new XMFLOAT4(Xp, 5, Zp, 0));
		XMStoreFloat3(&p1.posW, posW);

		XMVECTOR eyePosW = XMVectorSet(m_camera.GetPosition().x, m_camera.GetPosition().y, m_camera.GetPosition().z, 1);

		D3D11_MAPPED_SUBRESOURCE psPerFrameResource;
		ZeroMemory(&psPerFrameResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_psPerFrameConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &psPerFrameResource));

		//update the Frame data
		/*

			cbuffer PerFrameCB : register(b1)
			{
				DirectionalLight d;
				PointLight p;
				SpotLight s;
				float3 eyePosW;
			};

			struct DirectionalLight {
				float4 ambient;
				float4 diffuse;
				float4 specular;
				float3 dirW;
			};
		*/
		PerFrameCB* psFrameConstantBufferData = (PerFrameCB*)psPerFrameResource.pData;

		XMStoreFloat4(&psFrameConstantBufferData->dirLight.ambient, XMVectorSet(d1.ambient.x, d1.ambient.y, d1.ambient.z, d1.ambient.w));
		XMStoreFloat4(&psFrameConstantBufferData->dirLight.diffuse, XMVectorSet(d1.diffuse.x, d1.diffuse.y, d1.diffuse.z, d1.diffuse.w));
		XMStoreFloat4(&psFrameConstantBufferData->dirLight.specular, XMVectorSet(d1.specular.x, d1.specular.y, d1.specular.z, d1.specular.w));
		XMStoreFloat3(&psFrameConstantBufferData->dirLight.dirW, XMVectorSet(d1.dirW.x, d1.dirW.y, d1.dirW.z, 0));
		/*
			struct PointLight {
				float4 ambient;
				float4 diffuse;
				float4 specular;
				float3 posW;
				float range;
				float3 attenuation;
			};
		*/
		XMStoreFloat4(&psFrameConstantBufferData->pointLight.ambient, XMVectorSet(p1.ambient.x, p1.ambient.y, p1.ambient.z, p1.ambient.w));
		XMStoreFloat4(&psFrameConstantBufferData->pointLight.diffuse, XMVectorSet(p1.diffuse.x, p1.diffuse.y, p1.diffuse.z, p1.diffuse.w));
		XMStoreFloat4(&psFrameConstantBufferData->pointLight.specular, XMVectorSet(p1.specular.x, p1.specular.y, p1.specular.z, p1.specular.w));
		XMStoreFloat3(&psFrameConstantBufferData->pointLight.posW, XMVectorSet(p1.posW.x, p1.posW.y, p1.posW.z, 0));
		XMStoreFloat(&psFrameConstantBufferData->pointLight.range, XMVectorSet(p1.range, 0, 0, 0));
		XMStoreFloat3(&psFrameConstantBufferData->pointLight.attenuation, XMVectorSet(p1.attenuation.x, p1.attenuation.y, p1.attenuation.z, 0));
		/*
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
		*/
		XMStoreFloat4(&psFrameConstantBufferData->spotLight.ambient, XMVectorSet(s1.ambient.x, s1.ambient.y, s1.ambient.z, s1.ambient.w));
		XMStoreFloat4(&psFrameConstantBufferData->spotLight.diffuse, XMVectorSet(s1.diffuse.x, s1.diffuse.y, s1.diffuse.z, s1.diffuse.w));
		XMStoreFloat4(&psFrameConstantBufferData->spotLight.specular, XMVectorSet(s1.specular.x, s1.specular.y, s1.specular.z, s1.specular.w));
		XMStoreFloat3(&psFrameConstantBufferData->spotLight.posW, XMVectorSet(s1.posW.x, s1.posW.y, s1.posW.z, 0));
		XMStoreFloat(&psFrameConstantBufferData->spotLight.range, XMVectorSet(s1.range, 0, 0, 0));
		XMStoreFloat3(&psFrameConstantBufferData->spotLight.dirW, XMVectorSet(s1.dirW.x, s1.dirW.y, s1.dirW.z, 0));
		XMStoreFloat(&psFrameConstantBufferData->spotLight.spot, XMVectorSet(s1.spot, 0, 0, 0));
		XMStoreFloat3(&psFrameConstantBufferData->spotLight.attenuation, XMVectorSet(s1.attenuation.x, s1.attenuation.y, s1.attenuation.z, 0));

		XMStoreFloat3(&psFrameConstantBufferData->eyePosW, eyePosW);

		// enable GPU access
		m_d3dContext->Unmap(m_psPerFrameConstantBuffer.Get(), 0);
	}

	m_d3dAnnotation->EndEvent();
}


void LightDemoApp::RenderScene()
{
	m_d3dAnnotation->BeginEvent(L"render-scene");


	UINT stride = sizeof(mesh::MeshData::Vertex);
	UINT offset = 0;

	// clear the frame
	m_d3dContext->ClearDepthStencilView(m_depthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	m_d3dContext->ClearRenderTargetView(m_backBufferView.Get(), DirectX::Colors::Violet);

	// set the shaders and the input layout
	m_d3dContext->RSSetState(m_rasterizerState.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	//bind vs
	m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);

	//bind ps
	m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// set mesh buffers
	
	//bind cbuffer perframe
	m_d3dContext->PSSetConstantBuffers(PS_PERFRAME_CONSTANT_BUFFER_REGISTER, 1, m_psPerFrameConstantBuffer.GetAddressOf());
	m_d3dContext->VSSetConstantBuffers(VS_PEROBJECT_CONSTANT_BUFFER_REGISTER, 1, m_vsConstantBuffer.GetAddressOf());
	
	{	
		// ***************** SPHERE
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
			XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			PerObjectCB* constantBufferData = (PerObjectCB*)mappedResource.pData;

			XMStoreFloat4x4(&constantBufferData->WVP, GetWVPXMMATRIX(m_sphere.GetTransform()));
			XMStoreFloat4x4(&constantBufferData->WT, GetWTXMMATRIX(m_sphere.GetTransform()));
			XMStoreFloat4x4(&constantBufferData->W, m_sphere.GetTransform());
			
			m_d3dContext->Unmap(m_vsConstantBuffer.Get(), 0);
		}

		// draw sphere
		m_d3dContext->VSSetConstantBuffers  (VS_PEROBJECT_CONSTANT_BUFFER_REGISTER, 1, m_vsConstantBuffer.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers  (PS_PEROBJECT_CONSTANT_BUFFER_REGISTER, 1, m_sphere.d3dPSPerObjConstantBuffer.GetAddressOf());
		m_d3dContext->IASetVertexBuffers	(0, 1, m_sphere.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer		(m_sphere.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//m_d3dContext->Draw(m_sphere.mesh.vertices.size(), 0);
		m_d3dContext->DrawIndexed(UINT(m_sphere.mesh.indices.size()), 0, 0);
	}
	
	{ // ***************** PLANE

		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
			XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			PerObjectCB* constantBufferData = (PerObjectCB*)mappedResource.pData;

			XMStoreFloat4x4(&constantBufferData->WVP, GetWVPXMMATRIX(m_plane.GetTransform()));
			XMStoreFloat4x4(&constantBufferData->WT, GetWTXMMATRIX(m_plane.GetTransform()));
			XMStoreFloat4x4(&constantBufferData->W, m_plane.GetTransform());

			m_d3dContext->Unmap(m_vsConstantBuffer.Get(), 0);
		}
		// draw plane
		m_d3dContext->VSSetConstantBuffers  (VS_PEROBJECT_CONSTANT_BUFFER_REGISTER, 1, m_vsConstantBuffer.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers	(PS_PEROBJECT_CONSTANT_BUFFER_REGISTER, 1, m_plane.d3dPSPerObjConstantBuffer.GetAddressOf());
		m_d3dContext->IASetVertexBuffers	(0, 1, m_plane.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer		(m_plane.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//m_d3dContext->Draw(m_plane.mesh.vertices.size(), 0);
		m_d3dContext->DrawIndexed(UINT(m_plane.mesh.indices.size()), 0, 0);
	}
	
	{ // ***************** CUBE
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
			XTEST_D3D_CHECK(m_d3dContext->Map(m_vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			PerObjectCB* constantBufferData = (PerObjectCB*)mappedResource.pData;

			XMStoreFloat4x4(&constantBufferData->WVP, GetWVPXMMATRIX(m_cube.GetTransform()));
			XMStoreFloat4x4(&constantBufferData->WT, GetWTXMMATRIX(m_cube.GetTransform()));
			XMStoreFloat4x4(&constantBufferData->W, m_cube.GetTransform());
			
			m_d3dContext->Unmap(m_vsConstantBuffer.Get(), 0);
		}

		// draw cube
		m_d3dContext->VSSetConstantBuffers  (VS_PEROBJECT_CONSTANT_BUFFER_REGISTER, 1, m_vsConstantBuffer.GetAddressOf());
		m_d3dContext->PSSetConstantBuffers	(PS_PEROBJECT_CONSTANT_BUFFER_REGISTER, 1, m_cube.d3dPSPerObjConstantBuffer.GetAddressOf());
		m_d3dContext->IASetVertexBuffers	(0, 1, m_cube.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
		m_d3dContext->IASetIndexBuffer		(m_cube.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//m_d3dContext->Draw(m_cube.mesh.vertices.size(), 0);
		m_d3dContext->DrawIndexed(UINT(m_cube.mesh.indices.size()), 0, 0);
	}

	//present
	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}
