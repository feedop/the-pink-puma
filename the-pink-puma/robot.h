#pragma once
#include "dxApplication.h"
#include "mesh.h"

#include <array>
#include <ranges>

namespace mini::gk2
{
	class Robot
	{
	public:
		explicit Robot(const DxDevice& device, DirectX::XMFLOAT3 circleCenter, float circleRadius);

		inline auto GetDrawData() const
		{
			return std::ranges::views::zip(m_meshes, m_mtx);
		}

		inline auto GetPositionOnCircle() const
		{
			return m_positionOnCircle;
		}

		void Animate(std::array<float, 5>& angles, float angle);
		void Update(const std::array<float, 5>& angles);

	private:
		std::array<Mesh, 6> m_meshes;
		std::array<DirectX::XMFLOAT4X4, 6> m_mtx;
		std::array<DirectX::XMMATRIX, 5> m_tempMtx;

		const DirectX::XMMATRIX m_commonTransformation = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f)* DirectX::XMMatrixTranslation(0.0f, -1.5f, 0.0f);

		float m_circleAngle = 0;
		DirectX::XMVECTOR m_positionOnCircle;
		DirectX::XMVECTOR m_circleCenter;
		float m_circleRadius;
	};
}