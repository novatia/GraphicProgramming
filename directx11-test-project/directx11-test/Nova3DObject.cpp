#pragma once
#include "stdafx.h"
#include "mesh/mesh_format.h";
#include "Nova3DObject.h";

Nova3DObject::Nova3DObject()
{

}

DirectX::XMMATRIX Nova3DObject::GetTransform() {
	DirectX::XMMATRIX matRotateX;
	DirectX::XMMATRIX matRotateY;
	DirectX::XMMATRIX matRotateZ;
	DirectX::XMMATRIX matScale;
	DirectX::XMMATRIX matTranslate;

	DirectX::XMMATRIX transform;

	matRotateX = DirectX::XMMatrixRotationX(rotation.x);
	matRotateY = DirectX::XMMatrixRotationY(rotation.y);
	matRotateZ = DirectX::XMMatrixRotationZ(rotation.z);
	matScale = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	matTranslate = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	transform = matRotateX * matRotateY * matRotateZ * matScale * matTranslate;

	return transform;

}
