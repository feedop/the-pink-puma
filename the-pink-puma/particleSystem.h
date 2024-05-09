#pragma once
#include <DirectXMath.h>
#include <vector>
#include <random>
#include <d3d11.h>

namespace mini
{
	namespace gk2
	{
		struct ParticleVertex
		{
			DirectX::XMFLOAT3 Pos;
			float Age;
			float Angle;
			float Size;
			static const D3D11_INPUT_ELEMENT_DESC Layout[4];

			ParticleVertex() : Pos(0.0f, 0.0f, 0.0f), Age(0.0f), Angle(0.0f), Size(0.0f) { }
		};

		struct Particle
		{
			ParticleVertex Vertex;
			DirectX::XMFLOAT3 Velocity{ 0.0f, 0.0f, 0.0f };
		};

		class ParticleSystem
		{
		public:
			ParticleSystem() = default;

			ParticleSystem(ParticleSystem&& other) = default;

			ParticleSystem(DirectX::XMFLOAT3 emitterPosition);

			ParticleSystem& operator=(ParticleSystem&& other) = default;

			std::vector<ParticleVertex> Update(float dt, DirectX::XMFLOAT4 cameraPosition);

			inline size_t ParticleCount() const 
			{
				return m_particles.size();
			
			}
			inline void SetEmitterPos(DirectX::XMVECTOR emitterPosition)
			{
				XMStoreFloat3(&m_emitterPos, emitterPosition);
			}

			inline void Clear()
			{
				m_particles.clear();
			}

			inline static constexpr int MAX_PARTICLES = 2000;		//maximal number of particles in the system

		private:
			inline static constexpr DirectX::XMFLOAT3 EMITTER_DIR{ 0.0f, 1.0f, 0.0f };	//mean direction of particles' velocity
			inline static constexpr float TIME_TO_LIVE = 3.0f; //time of particle's life in seconds
			inline static constexpr float EMISSION_RATE = 400.0f; //number of particles to be born per second
			inline static constexpr float MIN_VELOCITY = 0.6f;	//minimal value of particle's velocity
			inline static constexpr float MAX_VELOCITY = 0.8f;	//maximal value of particle's velocity
			inline static constexpr float MIN_FORWARD_VELOCITY = 0.01f;	//minimal value of particle's velocity
			inline static constexpr float MAX_FORWARD_VELOCITY = 0.3f;	//maximal value of particle's velocity
			inline static constexpr float PARTICLE_SIZE = 0.02f;	//initial size of a particle
			inline static constexpr float PARTICLE_SCALE = 1.0f; //size += size*scale*dtime
			inline static constexpr float ACCELERATION = 0.003f;

			DirectX::XMFLOAT3 m_emitterPos;
			float m_particlesToCreate;

			std::vector<Particle> m_particles;

			std::default_random_engine m_random;

			DirectX::XMFLOAT3 RandomVelocity();
			Particle RandomParticle();
			static void UpdateParticle(Particle& p, float dt);
			std::vector<ParticleVertex> GetParticleVerts(DirectX::XMFLOAT4 cameraPosition);
		};
	}
}