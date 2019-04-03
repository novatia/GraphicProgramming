#pragma once
#include <application/directx_app.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include "Nova3DObject.h"

namespace xtest {
	namespace demo {

		class LightDemoApp : public application::DirectxApp, public input::MouseListener, public input::KeyboardListener
		{
		public:
			Nova3DObject m_sphere;
			Nova3DObject m_cube;
			Nova3DObject m_plane;

			/*struct DirectionalLight {
					float4 ambient;
					float4 diffuse;
					float4 specular;
					float3 dirW;
				};

				16byte
			*/
			struct DirectionalLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;

				DirectX::XMFLOAT3 dirW;
				float _explicit_pad_1;
			};


			/*struct SpotLight {
				float4 ambient;
				float4 diffuse;
				float4 specular;
				float3 posW;
				float range;
				float3 dirW;
				float spot;
				float3 attenuation;

				24byte
			};*/
			struct SpotLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				
				DirectX::XMFLOAT3 posW;
				float _explicit_pad_1;
				
				float range;
				float _explicit_pad_2;
				float _explicit_pad_3;
				float _explicit_pad_4;
				
				DirectX::XMFLOAT3 dirW;
				float _explicit_pad_5;

				float spot;
				float _explicit_pad_6;
				float _explicit_pad_7;
				float _explicit_pad_8;

				DirectX::XMFLOAT3  attenuation;
				float _explicit_pad_9;
			};

			/*
			struct PointLight {
				float4 ambient;
				float4 diffuse;
				float4 specular;
				float3 posW;
				float range;
				float3 attenuation;

				20byte
			};*/
			struct PointLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT3 posW;
				float _explicit_pad_1;

				float range;
				float _explicit_pad_2;
				float _explicit_pad_3;
				float _explicit_pad_4;

				DirectX::XMFLOAT3  attenuation;
				float _explicit_pad_5;
			};

			/*
				struct VertexIn
				{
					float3 POSITION_L : POSITION;
					float3 NORMAL_L : NORMAL_L;
					float3 TANGENT_U: TANGENT_U;
					float2 UV_L: UV_L;
				};
			*/
			struct VertexIn {
				DirectX::XMFLOAT3 positionL;
				DirectX::XMFLOAT3 normal;
				DirectX::XMFLOAT3 tangentU;
				DirectX::XMFLOAT2 uv;
			};

			struct PerObjectCB
			{
				DirectX::XMFLOAT4X4 W;
				DirectX::XMFLOAT4X4 WT;
				DirectX::XMFLOAT4X4 WVP;
				DirectX::XMFLOAT4X4 transform;
				Nova3DMaterial material;
			};

			struct PerFrameCB 
			{
				DirectionalLight dirLight;
				PointLight pointLight;
				SpotLight spotLight;
				DirectX::XMFLOAT3 eyePosW;
				float _explicit_pad_1;
			};

			struct RarelyChangedCB
			{
				DirectX::XMFLOAT4  useDPSLight;
			};

			DirectionalLight d1;
			SpotLight s1;
			PointLight p1;

			bool spotIsOn = true;
			bool directionalIsOn = true;
			bool pointIsOn = true;

			Nova3DMaterial metal;
			Nova3DMaterial concrete;

			LightDemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
			~LightDemoApp();

			LightDemoApp(LightDemoApp&&) = delete;
			LightDemoApp(const LightDemoApp&) = delete;
			LightDemoApp& operator=(LightDemoApp&&) = delete;
			LightDemoApp& operator=(const LightDemoApp&) = delete;


			virtual void Init() override;
			virtual void OnResized() override;
			virtual void UpdateScene(float deltaSeconds) override;
			virtual void RenderScene() override;

			virtual void OnWheelScroll(input::ScrollStatus scroll) override;
			virtual void OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos) override;
			virtual void OnKeyStatusChange(input::Key key, const input::KeyStatus& status) override;
		
		private:
			void InitLightsAndMaterials();
			void InitMeshes();
			void InitMatrices();
			void InitShaders();
			void InitBuffers();
			void InitRasterizerState();

			DirectX::XMMATRIX GetWVPXMMATRIX();
			DirectX::XMMATRIX GetWTXMMATRIX();
			DirectX::XMMATRIX GetWXMMATRIX();

			DirectX::XMFLOAT4X4 m_viewMatrix;
			DirectX::XMFLOAT4X4 m_worldMatrix;
			DirectX::XMFLOAT4X4 m_projectionMatrix;

			camera::SphericalCamera m_camera;

			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsConstantBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_psPerFrameConstantBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_psRarelyConstantBuffer;

			Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
			Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

		};

	} // demo
} // xtest

