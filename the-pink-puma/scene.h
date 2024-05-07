#pragma once
#include "dxApplication.h"
#include "mesh.h"
#include "particleSystem.h"
#include "robot.h"

namespace mini::gk2
{
	class Scene : public DxApplication
	{
	public:
		using Base = DxApplication;

		explicit Scene(HINSTANCE appInstance);

	protected:
		void Update(const Clock& dt) override;
		void Render() override;

	private:
#pragma region CONSTANTS
		inline static constexpr float TABLE_H = 1.0f;
		inline static constexpr float TABLE_TOP_H = 0.1f;
		inline static constexpr float TABLE_R = 1.5f;
		inline static constexpr unsigned int MAP_SIZE = 1024;
		inline static constexpr float LIGHT_NEAR = 0.35f;
		inline static constexpr float LIGHT_FAR = 5.5f;
		inline static constexpr float LIGHT_FOV_ANGLE = DirectX::XM_PI / 3.0f;

		inline static constexpr DirectX::XMFLOAT3 CIRCLE_CENTER = { -0.75f, -1.5f, 0.0f };
		inline static constexpr float CIRCLE_RADIUS = 0.2f;

#pragma endregion
		dx_ptr<ID3D11Buffer> m_cbWorldMtx, //vertex shader constant buffer slot 0
			m_cbProjMtx;	//vertex shader constant buffer slot 2 & geometry shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbViewMtx; //vertex shader constant buffer slot 1
		dx_ptr<ID3D11Buffer> m_cbSurfaceColor;	//pixel shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbLightPos; //pixel shader constant buffer slot 1

		Mesh m_wall; //uses m_wallsMtx[6]
		Mesh m_cylinder; //uses m_cylinderMtx
		dx_ptr<ID3D11Buffer> m_vbParticles;

		DirectX::XMFLOAT4X4 m_projMtx, m_wallsMtx[6], m_cylinderMtx, m_lightViewMtx[2], m_lightProjMtx;

		DirectX::XMFLOAT4 m_lightPos;

		dx_ptr<ID3D11SamplerState> m_sampler;

		dx_ptr<ID3D11ShaderResourceView> m_smokeTexture, m_opacityTexture;

		dx_ptr<ID3D11RasterizerState> m_rsCullNone;
		dx_ptr<ID3D11BlendState> m_bsAlpha;
		dx_ptr<ID3D11DepthStencilState> m_dssNoWrite;

		dx_ptr<ID3D11InputLayout> m_inputlayout, m_particleLayout;

		dx_ptr<ID3D11VertexShader> m_phongVS, m_particleVS;
		dx_ptr<ID3D11GeometryShader> m_particleGS;
		dx_ptr<ID3D11PixelShader> m_phongPS, m_particlePS;

		ParticleSystem m_particles;

		Robot m_robot;

		bool m_animateRobot = false;
		std::array<float, 5> m_robotAngles{};

		bool HandleKeyboardInput(double dt);

		void UpdateCameraCB(DirectX::XMMATRIX viewMtx);
		void UpdateCameraCB() { UpdateCameraCB(m_camera.getViewMatrix()); }
		void UpdateParticles(float dt);

		void DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx);
		void DrawParticles();
		void DrawTransparentObjects();
		void DrawRobot();

		void SetWorldMtx(DirectX::XMFLOAT4X4 mtx);
		void SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps);
		void SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList, const dx_ptr<ID3D11SamplerState>& sampler);
		void SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList) { SetTextures(std::move(resList), m_sampler); }

		void DrawScene();
	};
}