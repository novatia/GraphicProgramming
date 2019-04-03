#pragma once
#include "stdafx.h"
#include "mesh/mesh_format.h";

/*struct Material {
				float4 ambient;
				float4 diffuse;
				float4 specular;
			};
*/
struct Nova3DMaterial {
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
};

class Nova3DObject {
public:
	xtest::mesh::MeshData mesh;

	Microsoft::WRL::ComPtr<ID3D11Buffer> d3dVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> d3dIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> d3dPSPerObjConstantBuffer;

	Nova3DMaterial material;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;

	Nova3DObject();
	DirectX::XMMATRIX GetTransform();
};


