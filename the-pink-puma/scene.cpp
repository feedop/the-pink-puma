#include "scene.h"
#include <array>
#include "mesh.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;

Scene::Scene(HINSTANCE appInstance)
	: DxApplication(appInstance, 1280, 720, L"Pokój"),
	//Constant Buffers
	m_cbWorldMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbProjMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbViewMtx(m_device.CreateConstantBuffer<XMFLOAT4X4, 2>()),
	m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()),
	m_cbLightPos(m_device.CreateConstantBuffer<XMFLOAT4>()),

	//Textures
	m_smokeTexture(m_device.CreateShaderResourceView(L"resources/textures/smoke.png")),
	m_opacityTexture(m_device.CreateShaderResourceView(L"resources/textures/smokecolors.png")),

	// Robot
	m_robot(m_device, CIRCLE_CENTER, CIRCLE_RADIUS)
{
	//Projection matrix
	auto s = m_window.getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));
	UpdateBuffer(m_cbProjMtx, m_projMtx);
	UpdateCameraCB();

	//Sampler States
	SamplerDescription sd;

	// Create sampler with appropriate border color and addressing (border) and filtering (bilinear) modes
	sd.BorderColor[0] = 0;
	sd.BorderColor[1] = 0;
	sd.BorderColor[2] = 0;
	sd.BorderColor[3] = 0;
	sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	m_sampler = m_device.CreateSamplerState(sd);

	//Meshes
	vector<VertexPositionNormal> vertices;
	vector<unsigned short> indices;
	// TODO: Load robot
	m_wall = Mesh::Rectangle(m_device, 4.0f);
	m_cylinder = Mesh::Cylinder(m_device, 4, 9, 1.0f, 0.1f);
	m_mirror = Mesh::DoubleRect(m_device, 2.f * (CIRCLE_RADIUS + MIRROR_OFFSET));

	m_vbParticles = m_device.CreateVertexBuffer<ParticleVertex>(ParticleSystem::MAX_PARTICLES);

	//World matrix of all objects
	auto temp = XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	auto a = 0.f;
	for (auto i = 0U; i < 4U; ++i, a += XM_PIDIV2)
		XMStoreFloat4x4(&m_wallsMtx[i], temp * XMMatrixRotationY(a));
	XMStoreFloat4x4(&m_wallsMtx[4], temp * XMMatrixRotationX(XM_PIDIV2));
	XMStoreFloat4x4(&m_wallsMtx[5], temp * XMMatrixRotationX(-XM_PIDIV2));
	
	XMStoreFloat4x4(&m_cylinderMtx, XMMatrixRotationZ(XM_PIDIV2) * XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, -2.0f, 1.0f));

	XMStoreFloat4x4(&m_mirrorMtx, XMMatrixRotationY(XM_PIDIV2) * XMMatrixRotationZ(XM_PIDIV2 / 3.0f) * XMMatrixTranslation(CIRCLE_CENTER.x, CIRCLE_CENTER.y, CIRCLE_CENTER.z));

	// Light position
	m_lightPos = { -1.0f, 1.0f, -1.0f, 1.0f };

	//Render states
	RasterizerDescription rsDesc;
	rsDesc.CullMode = D3D11_CULL_NONE;
	m_rsCullNone = m_device.CreateRasterizerState(rsDesc);

	m_bsAlpha = m_device.CreateBlendState(BlendDescription::AlphaBlendDescription());
	DepthStencilDescription dssDesc;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_dssNoWrite = m_device.CreateDepthStencilState(dssDesc);

	auto vsCode = m_device.LoadByteCode(L"phongVS.cso");
	auto psCode = m_device.LoadByteCode(L"phongPS.cso");
	m_phongVS = m_device.CreateVertexShader(vsCode);
	m_phongPS = m_device.CreatePixelShader(psCode);
	m_inputlayout = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

	vsCode = m_device.LoadByteCode(L"particleVS.cso");
	psCode = m_device.LoadByteCode(L"particlePS.cso");
	auto gsCode = m_device.LoadByteCode(L"particleGS.cso");
	m_particleVS = m_device.CreateVertexShader(vsCode);
	m_particlePS = m_device.CreatePixelShader(psCode);
	m_particleGS = m_device.CreateGeometryShader(gsCode);
	m_particleLayout = m_device.CreateInputLayout<ParticleVertex>(vsCode);

	m_device.context()->IASetInputLayout(m_inputlayout.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//We have to make sure all shaders use constant buffers in the same slots!
	//Not all slots will be use by each shader
	ID3D11Buffer* vsb[] = { m_cbWorldMtx.get(),  m_cbViewMtx.get(), m_cbProjMtx.get() };
	m_device.context()->VSSetConstantBuffers(0, 3, vsb); //Vertex Shaders - 0: worldMtx, 1: viewMtx,invViewMtx, 2: projMtx, 3: tex1Mtx, 4: tex2Mtx
	m_device.context()->GSSetConstantBuffers(0, 1, vsb + 2); //Geometry Shaders - 0: projMtx
	ID3D11Buffer* psb[] = { m_cbSurfaceColor.get(), m_cbLightPos.get() };
	m_device.context()->PSSetConstantBuffers(0, 2, psb); //Pixel Shaders - 0: surfaceColor, 1: lightPos, 2: mapMtx
}

bool Scene::HandleKeyboardInput(double dt)
{
	static KeyboardState prevState;

	KeyboardState kstate;
	if (!m_keyboard.GetState(kstate))
		return false;

	const auto& forward = m_camera.getForwardDir();
	const auto& right = m_camera.getRightDir();

	bool flag = false;
	if (kstate.isKeyDown(17)) // w
	{
		m_camera.Move(forward * MOVEMENT_SPEED * dt);
	}
	if (kstate.isKeyDown(30)) // a
	{
		m_camera.Move(-right * MOVEMENT_SPEED * dt);
	}
	if (kstate.isKeyDown(31)) // s
	{
		m_camera.Move(-forward * MOVEMENT_SPEED * dt);
	}
	if (kstate.isKeyDown(32)) // d
	{
		m_camera.Move(right * MOVEMENT_SPEED * dt);
	}
	if (kstate.isKeyDown(16)) // q
	{
		m_camera.Move(XMVectorSet(0, -1, 0, 0) * MOVEMENT_SPEED * dt);
	}
	if (kstate.isKeyDown(18)) // e
	{
		m_camera.Move(XMVectorSet(0, 1, 0, 0) * MOVEMENT_SPEED * dt);
	}
	if (prevState.keyPressed(kstate, 46)) // c
	{
		m_animateRobot = !m_animateRobot;
		if (!m_animateRobot)
			m_particles.Clear();
	}

	prevState = kstate;

	if (m_animateRobot)
		return flag;

	for (int i = 0; i < 5; i++)
	{
		if (kstate.isKeyDown(19 + i)) // r + i
		{
			m_robotAngles[i] = XMScalarModAngle(m_robotAngles[i] + ROBOT_SPEED * dt);
			flag = true;
		}
		if (kstate.isKeyDown(33 + i)) // f + i
		{
			m_robotAngles[i] = XMScalarModAngle(m_robotAngles[i] - ROBOT_SPEED * dt);
			flag = true;
		}
	}

	return flag;
}

void Scene::UpdateCameraCB(XMMATRIX viewMtx)
{
	XMVECTOR det;
	XMMATRIX invViewMtx = XMMatrixInverse(&det, viewMtx);
	XMFLOAT4X4 view[2];
	XMStoreFloat4x4(view, viewMtx);
	XMStoreFloat4x4(view + 1, invViewMtx);
	UpdateBuffer(m_cbViewMtx, view);
}


void mini::gk2::Scene::UpdateParticles(float dt)
{
	auto verts = m_particles.Update(dt, m_camera.getCameraPosition());
	UpdateBuffer(m_vbParticles, verts);
}

void Scene::Update(const Clock& c)
{
	double dt = c.getFrameTime();
	HandleCameraInput(dt);

	if (m_animateRobot)
	{
		m_robot.Animate(m_robotAngles, dt * ROBOT_SPEED);
		m_particles.SetEmitterPos(m_robot.GetPositionOnCircle());
		UpdateParticles(dt);
	}

	if (HandleKeyboardInput(dt) || m_animateRobot)
	{
		m_robot.Update(m_robotAngles);
	}
}

void Scene::SetWorldMtx(DirectX::XMFLOAT4X4 mtx)
{
	UpdateBuffer(m_cbWorldMtx, mtx);
}

void mini::gk2::Scene::SetShaders(const dx_ptr<ID3D11VertexShader>& vs, const dx_ptr<ID3D11PixelShader>& ps)
{
	m_device.context()->VSSetShader(vs.get(), nullptr, 0);
	m_device.context()->PSSetShader(ps.get(), nullptr, 0);
}

void mini::gk2::Scene::SetTextures(std::initializer_list<ID3D11ShaderResourceView*> resList, const dx_ptr<ID3D11SamplerState>& sampler)
{
	m_device.context()->PSSetShaderResources(0, resList.size(), resList.begin());
	auto s_ptr = sampler.get();
	m_device.context()->PSSetSamplers(0, 1, &s_ptr);
}

void Scene::DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx)
{
	SetWorldMtx(worldMtx);
	m.Render(m_device.context());
}

void Scene::DrawParticles()
{
	//Set input layout, primitive topology, shaders, vertex buffer, and draw particles
	SetTextures({ m_smokeTexture.get(), m_opacityTexture.get() });
	m_device.context()->IASetInputLayout(m_particleLayout.get());
	SetShaders(m_particleVS, m_particlePS);
	m_device.context()->GSSetShader(m_particleGS.get(), nullptr, 0);
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	unsigned int stride = sizeof(ParticleVertex);
	unsigned int offset = 0;
	auto vb = m_vbParticles.get();
	m_device.context()->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	m_device.context()->Draw(m_particles.ParticleCount(), 0);

	//Reset layout, primitive topology and geometry shader
	m_device.context()->GSSetShader(nullptr, nullptr, 0);
	m_device.context()->IASetInputLayout(m_inputlayout.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Scene::DrawTransparentObjects()
{
	m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(m_dssNoWrite.get(), 0);
	
	if (m_animateRobot)
	{
		DrawParticles();
	}
	
	m_device.context()->OMSetBlendState(nullptr, nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(nullptr, 0);
}

void Scene::DrawRobot()
{
	for (const auto& item : m_robot.GetDrawData())
	{
		DrawMesh(std::get<0>(item), std::get<1>(item));
	}
}

void Scene::DrawScene()
{
	UpdateBuffer(m_cbSurfaceColor, XMFLOAT4{ 0.7f, 0.7f, 1.0f, 1.0f });
	for (auto& wallMtx : m_wallsMtx)
		DrawMesh(m_wall, wallMtx);

	UpdateBuffer(m_cbSurfaceColor, XMFLOAT4{ 0.7f, 0.2f, 0.7f, 1.0f });
	DrawMesh(m_cylinder, m_cylinderMtx);

	UpdateBuffer(m_cbSurfaceColor, XMFLOAT4{ 1.0f, 0.6f, 0.7f, 1.0f });
	DrawRobot();

	DrawMesh(m_mirror, m_mirrorMtx);

	m_device.context()->RSSetState(nullptr);

	DrawTransparentObjects();
}

void Scene::Render()
{
	Base::Render();

	// Set up view port of the appropriate size
	UpdateBuffer(m_cbLightPos, m_lightPos);
	UpdateBuffer(m_cbProjMtx, m_projMtx);
	UpdateCameraCB();

	// Bind m_lightMap and m_shadowMap textures then draw objects and particles using light&shadow pixel shader
	SetShaders(m_phongVS, m_phongPS);

	DrawScene();

	m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(m_dssNoWrite.get(), 0);
	DrawParticles();
	m_device.context()->OMSetBlendState(nullptr, nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(nullptr, 0);
}