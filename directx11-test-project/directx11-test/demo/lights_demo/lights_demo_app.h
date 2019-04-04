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

			struct DirectionalLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT4 dirW;
			};
		
			struct SpotLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT4 dirW;
				DirectX::XMFLOAT4 posW;
				DirectX::XMFLOAT4 attenuation;
				DirectX::XMFLOAT4 range;
				float spot;
				float _a0;
				float _a1;
				float _a2;
			};

			struct PointLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT4 posW;
				DirectX::XMFLOAT4 attenuation;
				DirectX::XMFLOAT4 range;
			};

			struct VertexIn {
				DirectX::XMFLOAT3 positionL;
				DirectX::XMFLOAT3 normal;
				DirectX::XMFLOAT3 tangentU;
				DirectX::XMFLOAT2 uv;
			};

			struct PerObjectCB
			{
				DirectX::XMFLOAT4X4 W;   //16
				DirectX::XMFLOAT4X4 WT;  //16
				DirectX::XMFLOAT4X4 WVP; //16
				Nova3DMaterial material; //16
			};

			struct PerFrameCB 
			{
				DirectX::XMFLOAT4 eyePosW; //4
				DirectionalLight dirLight; //16
				PointLight pointLight; //24
				SpotLight spotLight;//32
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

			DirectX::XMMATRIX GetWVPXMMATRIX(DirectX::XMMATRIX W);
			DirectX::XMMATRIX GetWTXMMATRIX(DirectX::XMMATRIX W);

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

