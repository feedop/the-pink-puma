#include "particleSystem.h"

#include <iterator>
#include <execution>

#include "dxDevice.h"
#include "exceptions.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;

const D3D11_INPUT_ELEMENT_DESC ParticleVertex::Layout[4] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

ParticleSystem::ParticleSystem(DirectX::XMFLOAT3 emitterPosition)
	: m_emitterPos(emitterPosition), m_particlesToCreate(0.0f), m_random(random_device{}())
{ }

vector<ParticleVertex> ParticleSystem::Update(float dt, DirectX::XMFLOAT4 cameraPosition)
{
	std::for_each(std::execution::par, m_particles.begin(), m_particles.end(), [&](auto& p)
	{
		UpdateParticle(p, dt);
	});

	size_t removeCount = std::count_if(std::execution::par, m_particles.begin(), m_particles.end(), [&](auto& p)
	{
		return p.Vertex.Age >= TIME_TO_LIVE;
	});

	m_particles.erase(m_particles.begin(), m_particles.begin() + removeCount);

	m_particlesToCreate += dt * EMISSION_RATE;
	while (m_particlesToCreate >= 1.0f)
	{
		--m_particlesToCreate;
		if (m_particles.size() < MAX_PARTICLES)
			m_particles.push_back(RandomParticle());
	}
	return GetParticleVerts(cameraPosition);
}

XMFLOAT3 ParticleSystem::RandomVelocity()
{
	static uniform_real_distribution<float> angleDist(0, XM_2PI);
	static uniform_real_distribution<float> velDist(MIN_VELOCITY, MAX_VELOCITY);
	static uniform_real_distribution<float> forwardVelDist(MIN_VELOCITY, MAX_VELOCITY);
	float angle = angleDist(m_random);
	auto len = velDist(m_random);
	auto forwardLen = forwardVelDist(m_random);
	// random velocity on a circle in the yz plane
	XMFLOAT3 v{ forwardLen, cos(angle), sin(angle) };

	// rorate by 30 degrees to align the aforementioned circle with the cutting surface
	auto velocity = XMVector3Transform(XMVector3Normalize(XMLoadFloat3(&v)), XMMatrixRotationZ(XM_PI / 6));
	
	velocity = len * XMVector3Normalize(velocity);
	XMStoreFloat3(&v, velocity);
	return v;
}

Particle ParticleSystem::RandomParticle()
{
	Particle p;
	p.Vertex.Pos = m_emitterPos;
	p.Vertex.Age = 0.0f;
	p.Vertex.Angle = 0.0f;
	p.Vertex.Size = PARTICLE_SIZE;
	p.Velocity = RandomVelocity();

	return p;
}

void ParticleSystem::UpdateParticle(Particle& p, float dt)
{
	static constexpr float eps = 1e-5;
	p.Velocity.y -= ACCELERATION;

	auto pos = XMLoadFloat3(&p.Vertex.Pos);
	auto vel = XMLoadFloat3(&p.Velocity);
	XMStoreFloat3(&p.Vertex.Pos, XMVectorAdd(pos, vel * dt));


	p.Vertex.Age += dt;
	p.Vertex.Size += PARTICLE_SCALE * PARTICLE_SIZE * dt;

	if (XMVectorGetX(XMVector3Length(vel)) < eps)
		return;

	auto dot = XMVectorGetX(XMVector3Dot({ 0, 1 ,0 }, XMVector3Normalize(vel)));
	auto angle = acos(dot);

	// This soultion assumes surface facing the x direction
	// A general solution is possible but this is good enough for now
	p.Vertex.Angle = XMVectorGetZ(vel) < 0 ? angle : -angle;
}

vector<ParticleVertex> ParticleSystem::GetParticleVerts(DirectX::XMFLOAT4 cameraPosition)
{
	XMFLOAT4 cameraTarget(0.0f, 0.0f, 0.0f, 1.0f);

	vector<ParticleVertex> vertices;
	vertices.reserve(m_particles.size());
	transform(m_particles.begin(), m_particles.end(), std::back_inserter(vertices), [](const Particle& p) { return p.Vertex; });
	XMVECTOR camPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR camDir = XMVectorSubtract(XMLoadFloat4(&cameraTarget), camPos);
	sort(vertices.begin(), vertices.end(), [camPos, camDir](auto& p1, auto& p2)
	{
		auto p1Pos = XMLoadFloat3(&(p1.Pos));
		p1Pos.m128_f32[3] = 1.0f;
		auto p2Pos = XMLoadFloat3(&(p2.Pos));
		p2Pos.m128_f32[3] = 1.0f;
		auto d1 = XMVector3Dot(p1Pos - camPos, camDir).m128_f32[0];
		auto d2 = XMVector3Dot(p2Pos - camPos, camDir).m128_f32[0];
		return d1 > d2;
	});

	return vertices;
}