#pragma once

#include <DirectXMath.h>

namespace mini
{
	class Camera
	{
	public:
		virtual ~Camera() { }
		virtual DirectX::XMMATRIX getViewMatrix() const = 0;
	};

	class FreeCamera : public Camera
	{
	public:
		explicit FreeCamera(const DirectX::XMVECTOR& position);
		
		DirectX::XMMATRIX getViewMatrix() const override;
		DirectX::XMFLOAT4 getCameraPosition() const;
		
		inline void Move(DirectX::XMFLOAT3 v) { Move(XMLoadFloat3(&v)); }
		void Move(DirectX::FXMVECTOR v);
		void Rotate(float dx, float dy);

		inline float getXAngle() const { return m_angleX; }
		inline float getYAngle() const { return m_angleY; }

	protected:
		DirectX::XMVECTOR m_position, m_forward = DirectX::XMVectorSet(0, 0, 1, 1), m_up = DirectX::XMVectorSet(0, 1, 0, 1), m_right = DirectX::XMVectorSet(1, 0, 0, 1);

	private:
		float m_angleX = 0, m_angleY = 0;
		
	};

	class FPSCamera : public FreeCamera
	{
	public:
		explicit FPSCamera(const DirectX::XMVECTOR& position)
			: FreeCamera(position)
		{ }

		/*Returns target's forward direction parallel to ground (XZ) plane*/
		DirectX::XMVECTOR getForwardDir() const;
		/*Returns target's right direction parallel to ground (XZ) plane*/
		DirectX::XMVECTOR getRightDir() const;
	};
}