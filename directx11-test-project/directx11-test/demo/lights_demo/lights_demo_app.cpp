#include "stdafx.h"
#include "lights_demo_app.h"
#include <file/file_utils.h>
#include <math/math_utils.h>
#include <service/locator.h>
#include "mesh/mesh_generator.h"

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
{}


LightDemoApp::~LightDemoApp()
{}

void LightDemoApp::Init()
{
	application::DirectxApp::Init();

	m_d3dAnnotation->BeginEvent(L"init-demo");

	InitLightsAndMaterials();
	InitMeshes();
	InitMatrices();
	InitShaders();
	InitBuffers();
	InitRasterizerState();

	service::Locator::GetMouse()->AddListener(this);
	service::Locator::GetKeyboard()->AddListener(this, { input::Key::F });

	m_d3dAnnotation->EndEvent();
}
void LightDemoApp::InitMeshes() {
	m_sphere.mesh = mesh::GenerateSphere(2.f, 40, 40);
	m_sphere.position = XMFLOAT3(-2,2,-2);

	m_plane.mesh = mesh::GeneratePlane(30.f, 30.f, 30, 30);
	m_plane.position = XMFLOAT3( 0 , 0, 0 );


}

void LightDemoApp::InitLightsAndMaterials() {
	//RED
	d1.ambient	= XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	d1.diffuse	= XMFLOAT4(128.0f, 0.0f, 0.0f, 0.0f);
	d1.specular = XMFLOAT4(255.0f, 255.0f, 255.0f, 255.0f);
	d1.dirW		= XMFLOAT3(-1,-1,0);
	
	//GREEN
	s1.ambient		= XMFLOAT4(0.0f, 55.0f, 0.0f, 255.0f);
	s1.attenuation	= XMFLOAT3(0.1f,0.1f,0.1f);
	s1.diffuse		= XMFLOAT4(0.0f, 255.0f, 0.0f, 255.0f);
	s1.dirW			= XMFLOAT3(1, 1, 0);
	s1.posW			= XMFLOAT3(0, 2, 0);
	s1.range		= 3.0f;
	s1.specular		= XMFLOAT4(255.0f, 255.0f, 255.0f, 255.0f);
	s1.spot			= 2.0f;

	//BLUE
	p1.ambient = XMFLOAT4(0.0f, 0.0f, 55.0f, 255.0f);;
	p1.attenuation = XMFLOAT3(0.1f, 0.1f, 0.1f);
	p1.diffuse = XMFLOAT4(0.0f, 0.0f, 255.0f, 255.0f);
	p1.posW = XMFLOAT3(2, 2, 0);
	p1.range = 3.0f;
	p1.specular = XMFLOAT4(255.0f, 255.0f, 255.0f, 255.0f);

	metal.ambient  = XMFLOAT4(98.0f, 75.0f, 0.0f, 255.0f);
	metal.diffuse  = XMFLOAT4(255.0f, 255.0f, 30.0f, 255.0f);
	metal.specular = XMFLOAT4(255.0f, 255.0f, 121.0f, 255.0f);
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
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, normal_offset, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, tangentU_offset, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, uv_offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	const int vs_size = vsByteCode.ByteSize();
	char* byteCode = new char[vs_size];
	
	memcpy(byteCode, vsByteCode.Data(), vs_size);
	XTEST_D3D_CHECK( m_d3dDevice->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), byteCode, vs_size, &m_inputLayout) );
}

void LightDemoApp::InitBuffers()
{
	//SPHERE START
	
	D3D11_BUFFER_DESC sphereVertexBufferDesc;
	sphereVertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	sphereVertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex))* m_sphere.mesh.vertices.size();
	sphereVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	sphereVertexBufferDesc.CPUAccessFlags = 0;
	sphereVertexBufferDesc.MiscFlags = 0;
	sphereVertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA sphereVertexInitData;
	sphereVertexInitData.pSysMem = &m_sphere.mesh.vertices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&sphereVertexBufferDesc, &sphereVertexInitData, &m_sphere.d3dVertexBuffer));


	D3D11_BUFFER_DESC sphereIndexBufferDesc;
	sphereIndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	sphereIndexBufferDesc.ByteWidth = sizeof(uint32) * m_sphere.mesh.indices.size();;
	sphereIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	sphereIndexBufferDesc.CPUAccessFlags = 0;
	sphereIndexBufferDesc.MiscFlags = 0;
	sphereIndexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA sphereIndexInitdata;
	sphereIndexInitdata.pSysMem = &m_sphere.mesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&sphereIndexBufferDesc, &sphereIndexInitdata, &m_sphere.d3dIndexBuffer));
	//SPHERE END


	//PLANE START
	D3D11_BUFFER_DESC planeVertexBufferDesc;
	planeVertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	planeVertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex))* m_plane.mesh.vertices.size();
	planeVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	planeVertexBufferDesc.CPUAccessFlags = 0;
	planeVertexBufferDesc.MiscFlags = 0;
	planeVertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA planeVertexInitData;
	planeVertexInitData.pSysMem = &m_plane.mesh.vertices[0];

	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&planeVertexBufferDesc, &planeVertexInitData, &m_plane.d3dVertexBuffer));




	D3D11_BUFFER_DESC planeIndexBufferDesc;
	planeIndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	planeIndexBufferDesc.ByteWidth = sizeof(uint32) * m_plane.mesh.indices.size();;
	planeIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	planeIndexBufferDesc.CPUAccessFlags = 0;
	planeIndexBufferDesc.MiscFlags = 0;
	planeIndexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA planeIndexInitdata;
	planeIndexInitdata.pSysMem = &m_plane.mesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&planeIndexBufferDesc, &planeIndexInitdata, &m_plane.d3dIndexBuffer));
	//PLANE END

	//BOX START
	D3D11_BUFFER_DESC cubeVertexBufferDesc;
	cubeVertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	cubeVertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex))* m_cube.mesh.vertices.size();
	cubeVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	cubeVertexBufferDesc.CPUAccessFlags = 0;
	cubeVertexBufferDesc.MiscFlags = 0;
	cubeVertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cubeVertexInitData;
	cubeVertexInitData.pSysMem = &m_cube.mesh.vertices[0];

	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&cubeVertexBufferDesc, &cubeVertexInitData, &m_cube.d3dVertexBuffer));
	D3D11_BUFFER_DESC cubeIndexBufferDesc;
	cubeIndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	cubeIndexBufferDesc.ByteWidth = sizeof(uint32) * m_cube.mesh.indices.size();;
	cubeIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	cubeIndexBufferDesc.CPUAccessFlags = 0;
	cubeIndexBufferDesc.MiscFlags = 0;
	cubeIndexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cubeIndexInitdata;
	cubeIndexInitdata.pSysMem = &m_cube.mesh.indices[0];
	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&cubeIndexBufferDesc, &cubeIndexInitdata, &m_cube.d3dIndexBuffer));
	//BOX END



	// constant buffer to update the Vertex Shader "PerObjectCB" constant buffer:
	/*
		cbuffer PerObjectCB : register(b0)
		{
			float4x4 W;
			float4x4 WT;
			float4x4 WVP;
			Material material;
		};
	*/
	D3D11_BUFFER_DESC vsConstantBufferDesc;
	vsConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	vsConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
	vsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vsConstantBufferDesc.MiscFlags = 0;
	vsConstantBufferDesc.StructureByteStride = 0;

	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&vsConstantBufferDesc, nullptr, &m_vsConstantBuffer));


	/*
		cbuffer PerObjectCB : register(b0)
		{
			float4x4 W;
			float4x4 WT;
			float4x4 WVP;
			Material material;
		};
	*/
	D3D11_BUFFER_DESC psPerObjConstantBufferDesc;
	psPerObjConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	psPerObjConstantBufferDesc.ByteWidth = sizeof(PerObjectCB);
	psPerObjConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psPerObjConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psPerObjConstantBufferDesc.MiscFlags = 0;
	psPerObjConstantBufferDesc.StructureByteStride = 0;

	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&psPerObjConstantBufferDesc, nullptr, &m_psPerObjConstantBuffer));

	/*
	cbuffer PerFrameCB : register(b1)
	{
		DirectionalLight d;
		PointLight p;
		SpotLight s;
		float3 eyePosW;
	};*/
	D3D11_BUFFER_DESC psPerFrameConstantBufferDesc;
	psPerFrameConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	psPerFrameConstantBufferDesc.ByteWidth = sizeof(PerFrameCB);
	psPerFrameConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psPerFrameConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psPerFrameConstantBufferDesc.MiscFlags = 0;
	psPerFrameConstantBufferDesc.StructureByteStride = 0;

	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&psPerFrameConstantBufferDesc, nullptr, &m_psPerFrameConstantBuffer));

	/*
	cbuffer RarelyChangedCB : register(b2)
	{
		bool useDLight;
		bool usePLight;
		bool useSLight;
	};
	*/

	D3D11_BUFFER_DESC psRarelyConstantBufferDesc;
	psRarelyConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // this buffer needs to be updated every frame
	psRarelyConstantBufferDesc.ByteWidth = sizeof(RarelyChangedCB);
	psRarelyConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psRarelyConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psRarelyConstantBufferDesc.MiscFlags = 0;
	psRarelyConstantBufferDesc.StructureByteStride = 0;

	XTEST_D3D_CHECK(m_d3dDevice->CreateBuffer(&psRarelyConstantBufferDesc, nullptr, &m_psRarelyConstantBuffer));
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
	XTEST_ASSERT(key == input::Key::F); // the only key registered for this listener

	// re-frame the cube when F key is pressed
	if (status.isDown)
	{
		m_camera.SetPivot({ 0.f, 0.f, 0.f });
	}
}


void LightDemoApp::UpdateScene(float deltaSeconds)
{
	XTEST_UNUSED_VAR(deltaSeconds);

	XMMATRIX W = XMLoadFloat4x4(&m_worldMatrix);
	XMStoreFloat4x4(&m_worldMatrix, W);

	// create the model-view-projection matrix
	XMMATRIX V = m_camera.GetViewMatrix();
	XMStoreFloat4x4(&m_viewMatrix, V);

	// create projection matrix
	XMMATRIX P = XMLoadFloat4x4(&m_projectionMatrix);
	XMMATRIX WVP = W * V * P;

	// matrices must be transposed since HLSL use column-major ordering.
	WVP = XMMatrixTranspose(WVP);

	XMMATRIX WT = XMMatrixInverse(NULL, WVP);
	WT = XMMatrixTranspose(WT);


	m_d3dAnnotation->BeginEvent(L"update-constant-buffer");

	// load the constant buffer data in the gpu
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory (&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK (m_d3dContext->Map(m_vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		PerObjectCB* constantBufferData = (PerObjectCB*)mappedResource.pData;

		//update the data
		XMStoreFloat4x4(&constantBufferData->WVP, WVP);
		XMStoreFloat4x4(&constantBufferData->WT, WT);
		XMStoreFloat4x4(&constantBufferData->W, W);

		// enable GPU access

		m_d3dContext->Unmap(m_vsConstantBuffer.Get(), 0);


		//OBJECT MATERIALS
		D3D11_MAPPED_SUBRESOURCE psPerObjResource;
		ZeroMemory(&psPerObjResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_psPerObjConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &psPerObjResource));
		PerObjectCB* psConstantBufferData = (PerObjectCB*)psPerObjResource.pData;

		//update the data
		XMStoreFloat4x4(&psConstantBufferData->WVP, WVP);
		XMStoreFloat4x4(&psConstantBufferData->WT, WT);
		XMStoreFloat4x4(&psConstantBufferData->W, W);
		XMStoreFloat4(&psConstantBufferData->material.ambient, XMVectorSet(metal.ambient.x, metal.ambient.y, metal.ambient.z, metal.ambient.w));

		// enable GPU access
		m_d3dContext->Unmap(m_psPerObjConstantBuffer.Get(), 0);


		//LIGHTS
		D3D11_MAPPED_SUBRESOURCE psPerFrameResource;
		ZeroMemory(&psPerFrameResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_psPerFrameConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &psPerFrameResource));
		PerFrameCB* psFrameConstantBufferData = (PerFrameCB*)psPerFrameResource.pData;

		//update the data
		/*
		DirectionalLight dirLight;
		PointLight pointLight;
		SpotLight spotLight;
		DirectX::XMFLOAT3 eyePosW;
		*/
		XMStoreFloat4(&psFrameConstantBufferData->dirLight.ambient , XMVectorSet(d1.ambient.x, d1.ambient.y, d1.ambient.z, d1.ambient.w ) );
		XMStoreFloat4(&psFrameConstantBufferData->dirLight.diffuse,  XMVectorSet(d1.diffuse.x, d1.diffuse.y, d1.diffuse.z, d1.diffuse.w));
		XMStoreFloat3(&psFrameConstantBufferData->dirLight.dirW, XMVectorSet( d1.dirW.x, d1.dirW.y, d1.dirW.z, 0 ));
		XMStoreFloat4(&psFrameConstantBufferData->dirLight.specular, XMVectorSet(d1.specular.x, d1.specular.y, d1.specular.z, d1.specular.w));



		// enable GPU access
		m_d3dContext->Unmap(m_psPerFrameConstantBuffer.Get(), 0);


		D3D11_MAPPED_SUBRESOURCE psRarelyResource;
		ZeroMemory(&psRarelyResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		// disable gpu access
		XTEST_D3D_CHECK(m_d3dContext->Map(m_psRarelyConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &psRarelyResource));
		RarelyChangedCB* psRarelyConstantBufferData = (RarelyChangedCB*)psRarelyResource.pData;

		//update the data
		
		// enable GPU access
		m_d3dContext->Unmap(m_psRarelyConstantBuffer.Get(), 0);
	}

	m_d3dAnnotation->EndEvent();
}


void LightDemoApp::RenderScene()
{
	m_d3dAnnotation->BeginEvent(L"render-scene");

	// clear the frame
	m_d3dContext->ClearDepthStencilView(m_depthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	m_d3dContext->ClearRenderTargetView(m_backBufferView.Get(), DirectX::Colors::Black);

	// set the shaders and the input layout
	m_d3dContext->RSSetState(m_rasterizerState.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// bind the constant data to the vertex shader
	UINT bufferRegister = 0; // PerObjectCB was defined as register 0 inside the vertex shader file
	m_d3dContext->VSSetConstantBuffers(bufferRegister, 1, m_vsConstantBuffer.GetAddressOf());

	bufferRegister = 0; // PerObjectCB was defined as register 0 inside the vertex shader file
	m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, m_psPerObjConstantBuffer.GetAddressOf());

	bufferRegister = 1; // PerObjectCB was defined as register 0 inside the vertex shader file
	m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, m_psPerFrameConstantBuffer.GetAddressOf());

	bufferRegister =2; // PerObjectCB was defined as register 0 inside the vertex shader file
	m_d3dContext->PSSetConstantBuffers(bufferRegister, 1, m_psRarelyConstantBuffer.GetAddressOf());

	// set mesh buffers
	UINT stride = sizeof(mesh::MeshData::Vertex);
	UINT offset = 0;
	
	// draw sphere
	m_d3dContext->IASetVertexBuffers(0, 1, m_sphere.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_sphere.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_d3dContext->Draw(m_sphere.mesh.vertices.size(), 0);
	m_d3dContext->DrawIndexed (UINT(m_sphere.mesh.indices.size()), 0, 0);

	// draw plane
	m_d3dContext->IASetVertexBuffers(0, 1, m_plane.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_plane.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_d3dContext->Draw(m_plane.mesh.vertices.size(), 0);
	m_d3dContext->DrawIndexed(UINT(m_plane.mesh.indices.size()), 0, 0);

	
	// draw cube
	m_d3dContext->IASetVertexBuffers(0, 1, m_cube.d3dVertexBuffer.GetAddressOf(), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_cube.d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_d3dContext->Draw(m_cube.mesh.vertices.size(), 0);
	m_d3dContext->DrawIndexed(UINT(m_cube.mesh.indices.size()), 0, 0);
	
	//present
	XTEST_D3D_CHECK(m_swapChain->Present(0, 0));

	m_d3dAnnotation->EndEvent();
}

XMFLOAT4 LightDemoApp::PhongLighting( Nova3DMaterial M, XMFLOAT4 LColor, XMFLOAT3  N, XMFLOAT3  L, XMFLOAT3  V, XMFLOAT3  R)
{
/*	DirectX::XMFLOAT4 Ia = M.Ka + ambientLight;
	DirectX::XMFLOAT4 Id = M.Kd + N*L;
	DirectX::XMFLOAT4 Is = M.Ks * pow(saturate(dot(R, V)), M.A);
	
	return Ia + (Id + Is) * LColor;
	*/

	return XMFLOAT4{0.f,0.f,0.f,0.f};
}

XMFLOAT4 LightDemoApp::calcBlinnPhongLighting(Nova3DMaterial M, DirectX::XMFLOAT4 LColor, DirectX::XMFLOAT3 N, DirectX::XMFLOAT3 L, DirectX::XMFLOAT3 H) {
	/*DirectX::XMFLOAT4 Ia = M.Ka * ambientLight;
	DirectX::XMFLOAT4 Id = M.Kd * saturate(dot(N, L));
	DirectX::XMFLOAT4 Is = M.Ks * pow(saturate(dot(N, H)), M.A);

	return Ia + (Id + Is) * LColor;
	*/

	return XMFLOAT4{ 0.f,0.f,0.f,0.f };
}

