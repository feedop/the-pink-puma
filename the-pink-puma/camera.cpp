#include "camera.h"

using namespace mini;
using namespace DirectX;

FreeCamera::FreeCamera(const XMVECTOR& position) : m_position(position)
{
	
}

XMMATRIX FreeCamera::getViewMatrix() const
{
	return XMMatrixLookToLH(m_position, m_forward, m_up);
}

XMVECTOR FPSCamera::getForwardDir() const
{
	auto forward = XMVectorSet(0, 0, 1, 0);
	return XMVector3TransformNormal(forward, XMMatrixRotationY(getYAngle()));
}

XMVECTOR FPSCamera::getRightDir() const
{
	auto right = XMVectorSet(1, 0, 0, 0);
	return XMVector3TransformNormal(right, XMMatrixRotationY(getYAngle()));
}

void FreeCamera::Move(FXMVECTOR v)
{
	m_position += v;
}

void FreeCamera::Rotate(float dx, float dy)
{
	m_angleX = XMScalarModAngle(m_angleX + dx);
	m_angleY = XMScalarModAngle(m_angleY + dy);

	if (m_angleX < -XM_PIDIV2)
		m_angleX = -XM_PIDIV2;

	if (m_angleX > XM_PIDIV2)
		m_angleX = XM_PIDIV2;

	auto rotationX = XMMatrixRotationX(getXAngle());
	auto rotationY = XMMatrixRotationY(getYAngle());

	auto up = XMVectorSet(0, 1, 0, 0);
	m_up = XMVector3TransformNormal(up, rotationX * rotationY);

	auto right = XMVectorSet(1, 0, 0, 0);
	m_right = XMVector3TransformNormal(right, rotationX * rotationY);

	auto forward = XMVectorSet(0, 0, 1, 0);

	m_forward = XMVector3TransformNormal(forward, rotationX * rotationY);
}

DirectX::XMFLOAT4 FreeCamera::getCameraPosition() const
{
	XMFLOAT3 res(0.0f, 0.0f, 0.0f);
	XMStoreFloat3(&res, m_position);
	return XMFLOAT4(res.x, res.y, res.z, 1.0f);
}

