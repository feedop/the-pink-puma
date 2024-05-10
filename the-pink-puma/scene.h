#pragma once
#include "dxApplication.h"
#include "mesh.h"
#include "meshWithAdjacency.h"
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
		inline static constexpr DirectX::XMFLOAT3 CIRCLE_CENTER = { -0.75f, -1.5f, 0.0f };
		inline static constexpr float CIRCLE_RADIUS = 0.2f;
		inline static constexpr float MIRROR_OFFSET = 0.1f;

		static const unsigned int BS_MASK;

#pragma endregion
		dx_ptr<ID3D11Buffer> m_cbWorldMtx, //vertex shader constant buffer slot 0
			m_cbProjMtx;	//vertex shader constant buffer slot 2 & geometry shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbViewMtx; //vertex shader constant buffer slot 1
		dx_ptr<ID3D11Buffer> m_cbSurfaceColor;	//pixel shader constant buffer slot 0
		dx_ptr<ID3D11Buffer> m_cbLightPos; //pixel shader constant buffer slot 1

		Mesh m_wall; //uses m_wallsMtx[6]
		Mesh m_cylinder; //uses m_cylinderMtx
		Mesh m_mirror; // uses m_frontMirrorObjectMtx and m_backMirrorObjectMtx

		dx_ptr<ID3D11Buffer> m_vbParticles;

		DirectX::XMFLOAT4X4 m_projMtx,
			m_wallsMtx[6],
			m_cylinderMtx,
			m_lightViewMtx[2],
			m_lightProjMtx,
			m_frontMirrorObjectMtx,
			m_backMirrorObjectMtx,
			m_frontMirrorViewMtx,
			m_backMirrorViewMtx;

		DirectX::XMFLOAT4 m_lightPos;

		dx_ptr<ID3D11SamplerState> m_sampler;

		dx_ptr<ID3D11ShaderResourceView> m_smokeTexture, m_opacityTexture;

		dx_ptr<ID3D11RasterizerState> m_rsCullNone;
		dx_ptr<ID3D11BlendState> m_bsAlpha;
		dx_ptr<ID3D11DepthStencilState> m_dssNoWrite;
		//Rasterizer state used to define front faces as counter-clockwise, used when drawing mirrored scene
		dx_ptr<ID3D11RasterizerState> m_rsCCW;
		dx_ptr<ID3D11RasterizerState> m_rsCCW_backSh;
		dx_ptr<ID3D11RasterizerState> m_rsCCW_frontSh;

		//dx_ptr<ID3D11BlendState> m_bsAlpha;
		dx_ptr<ID3D11BlendState> m_bsAlphaInv;
		dx_ptr<ID3D11BlendState> m_bsAdd;

		//Depth stencil state used to fill the stencil buffer
		dx_ptr<ID3D11DepthStencilState> m_dssStencilWrite;
		//Depth stencil state used to perform stencil test when drawing mirrored scene
		dx_ptr<ID3D11DepthStencilState> m_dssStencilTest;

		dx_ptr<ID3D11DepthStencilState> m_dssNoDepthWrite;
		dx_ptr<ID3D11DepthStencilState> m_dssDepthWrite;
		//Depth stencil state used to fill the stencil buffer
		//dx_ptr<ID3D11DepthStencilState> m_dssStencilWrite;
		dx_ptr<ID3D11DepthStencilState> m_dssStencilWriteSh;
		//Depth stencil state used to perform stencil test when drawing mirrored scene
		//dx_ptr<ID3D11DepthStencilState> m_dssStencilTest;
		dx_ptr<ID3D11DepthStencilState> m_dssStencilTestSh;
		dx_ptr<ID3D11DepthStencilState> m_dssStencilTestGreaterSh;
		//Depth stencil state used to perform stencil test when drawing mirrored billboards without writing to the depth buffer
		dx_ptr<ID3D11DepthStencilState> m_dssStencilTestNoDepthWrite;

		dx_ptr<ID3D11InputLayout> m_inputlayout, m_particleLayout, m_silhouetteLayout;

		dx_ptr<ID3D11VertexShader> m_phongVS, m_particleVS, m_silhouetteVS;
		dx_ptr<ID3D11GeometryShader> m_particleGS, m_silhouetteGS;
		dx_ptr<ID3D11PixelShader> m_phongPS, m_particlePS, m_silhouettePS;

		ParticleSystem m_particles;

		Robot m_robot;

		bool m_animateRobot = false;
		std::array<float, 5> m_robotAngles{};

		bool HandleKeyboardInput(double dt);

		void UpdateCameraCB(DirectX::XMMATRIX viewMtx);
		void UpdateCameraCB() { UpdateCameraCB(m_camera.getViewMatrix()); }
		void UpdateParticles(float dt);

		void CreateRenderStates();

		void DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx);
		void DrawMeshWithAdjency(const MeshWithAdjency& m, const DirectX::XMFLOAT4X4& worldMtx);
		void DrawParticles();
		void DrawTransparentObjects();
		void DrawRobot();
		void DrawMirror();
		void DrawMirroredWorld(int i);
		void DrawSilhouette();
		void RenderShadow();

		void SetWorldMtx(DirectX::XMFLOAT4X4 mtx);
		void SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps);
		void SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList, const dx_ptr<ID3D11SamplerState>& sampler);
		void SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList) { SetTextures(std::move(resList), m_sampler); }

		void DrawScene(bool drawRobot = true);
	};
}