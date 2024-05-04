#include "robot.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;

Robot::Robot(const DxDevice& device)
{
	m_meshes[0] = Mesh::LoadMesh(device, L"resources/meshes/mesh1.txt");
	m_meshes[1] = Mesh::LoadMesh(device, L"resources/meshes/mesh2.txt");
	m_meshes[2] = Mesh::LoadMesh(device, L"resources/meshes/mesh3.txt");
	m_meshes[3] = Mesh::LoadMesh(device, L"resources/meshes/mesh4.txt");
	m_meshes[4] = Mesh::LoadMesh(device, L"resources/meshes/mesh5.txt");
	m_meshes[5] = Mesh::LoadMesh(device, L"resources/meshes/mesh6.txt");

	for (int i = 0; i < 6; i++)
	{
		XMStoreFloat4x4(&m_mtx[i], XMMatrixIdentity());
	}
}

void Robot::Update(double dt)
{

}
