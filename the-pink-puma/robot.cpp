#include "robot.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;

Robot::Robot(const DxDevice& device, XMFLOAT3 circleCenter, float circleRadius) : m_circleRadius(circleRadius)
{
	m_circleCenter = XMLoadFloat3(&circleCenter);

	m_meshes[0] = MeshWithAdjency::LoadMesh(device, L"resources/meshes/mesh1.txt");
	m_meshes[1] = MeshWithAdjency::LoadMesh(device, L"resources/meshes/mesh2.txt");
	m_meshes[2] = MeshWithAdjency::LoadMesh(device, L"resources/meshes/mesh3.txt");
	m_meshes[3] = MeshWithAdjency::LoadMesh(device, L"resources/meshes/mesh4.txt");
	m_meshes[4] = MeshWithAdjency::LoadMesh(device, L"resources/meshes/mesh5.txt");
	m_meshes[5] = MeshWithAdjency::LoadMesh(device, L"resources/meshes/mesh6.txt");

	XMStoreFloat4x4(&m_mtx[0], m_commonTransformation);
	Update({ 0,0,0,0,0 });
}

void Robot::Animate(std::array<float, 5>& angles, float angle)
{
	// point on circle
	m_circleAngle = XMScalarModAngle(m_circleAngle + angle);

	m_positionOnCircle = (XMVectorSet(XMScalarCos(m_circleAngle) * m_circleRadius, XMScalarSin(m_circleAngle) * m_circleRadius, 0, 1));
	m_positionOnCircle = XMVector4Transform(m_positionOnCircle, XMMatrixRotationY(XM_PIDIV2) * XMMatrixRotationZ(XM_PI / 6) * XMMatrixTranslationFromVector(m_circleCenter));
	XMVECTOR pos = XMVector4Transform(m_positionOnCircle, XMMatrixInverse(nullptr, m_commonTransformation));
	XMVECTOR normal = XMVector4Normalize(XMVectorSet(2, 1, 0, 0));

	// inverse kinematics
	float l1 = .91f, l2 = .81f, l3 = .33f, dy = .27f, dz = .26f;

	XMVECTOR pos1v = pos + normal * l3;
	XMFLOAT3 pos1;
	XMStoreFloat3(&pos1, pos1v);

	float e = sqrtf(pos1.z * pos1.z + pos1.x * pos1.x - dz * dz);
	angles[0] = atan2(pos1.z, -pos1.x) + atan2(dz, e);
	XMFLOAT3 pos2(e, pos1.y - dy, .0f);
	angles[2] = -acosf(min(1.0f, (pos2.x * pos2.x + pos2.y * pos2.y - l1 * l1 - l2 * l2)
				/ (2.0f * l1 * l2)));
	float k = l1 + l2 * cosf(angles[2]), l = l2 * sinf(angles[2]);
	angles[1] = -atan2(pos2.y, sqrtf(pos2.x * pos2.x + pos2.z * pos2.z)) - atan2(l, k);
	XMVECTOR normal1v = XMVector4Transform(normal, XMMatrixRotationY(-angles[0]) * XMMatrixRotationZ(-(angles[1] + angles[2])));
	XMFLOAT3 normal1;
	XMStoreFloat3(&normal1, normal1v);

	angles[4] = acosf(normal1.x);
	angles[3] = atan2(normal1.z, normal1.y);
}

void Robot::Update(const std::array<float, 5>& angles)
{
	// update matrices
	m_tempMtx[0] = XMMatrixRotationY(angles[0]);
	m_tempMtx[1] = XMMatrixTranslation(0, -0.27f, 0) * XMMatrixRotationZ(angles[1]) * XMMatrixTranslation(0, 0.27f, 0) * m_tempMtx[0];
	m_tempMtx[2] = XMMatrixTranslation(0.91f, -0.27f, 0) * XMMatrixRotationZ(angles[2]) * XMMatrixTranslation(-0.91f, 0.27f, 0) * m_tempMtx[1];
	m_tempMtx[3] = XMMatrixTranslation(0, -0.27f, 0.26f) * XMMatrixRotationX(angles[3]) * XMMatrixTranslation(0, 0.27f, -0.26f) * m_tempMtx[2];
	m_tempMtx[4] = XMMatrixTranslation(1.72f, -0.27f, 0) * XMMatrixRotationZ(angles[4]) * XMMatrixTranslation(-1.72f, 0.27f, 0) * m_tempMtx[3];

	for (int i = 1; i < 6; i++)
	{
		XMStoreFloat4x4(&m_mtx[i], m_tempMtx[i - 1] * m_commonTransformation);
	}
}
