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
		explicit Robot(const DxDevice& device);

		void Update(double dt);

		inline auto getDrawData() const
		{
			return std::ranges::views::zip(m_meshes, m_mtx);
		}

	private:
		std::array<Mesh, 6> m_meshes;
		std::array<DirectX::XMFLOAT4X4, 6> m_mtx;
	};
}