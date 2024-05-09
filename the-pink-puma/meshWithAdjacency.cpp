#include "meshWithAdjacency.h"
#include <fstream>
#include <map>

using namespace std;


mini::MeshWithAdjency::MeshWithAdjency(Mesh&& simpleMesh, dx_ptr<ID3D11Buffer>&& adjencyIndices, unsigned int adjencyIndexCount):
	Mesh(std::move(simpleMesh)), m_adjencyIndexBuffer(std::move(adjencyIndices)), m_adjencyIndexCount(adjencyIndexCount)
{
}


void mini::MeshWithAdjency::RenderWithAdjency(const dx_ptr<ID3D11DeviceContext>& context) const
{
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ);
    context->IASetIndexBuffer(m_adjencyIndexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
    context->IASetVertexBuffers(0, m_vertexBuffers.size(), m_vertexBuffers.data(), m_strides.data(), m_offsets.data());
    context->DrawIndexed(m_adjencyIndexCount, 0, 0);
}


mini::MeshWithAdjency mini::MeshWithAdjency::LoadMesh(const DxDevice& device, const std::wstring& meshPath)
{
	ifstream input;
	// In general we shouldn't throw exceptions on end-of-file,
	// however, in case of this file format if we reach the end
	// of a file before we read all values, the file is
	// ill-formated and we would need to throw an exception anyway
	input.exceptions(ios::badbit | ios::failbit | ios::eofbit);
	input.open(meshPath);

	int pn, vn, in, en;

	input >> pn;
	vector< DirectX::XMFLOAT3> positions(pn);
	for (auto i = 0; i < pn; ++i)
		input >> positions[i].x >> positions[i].y >> positions[i].z;

	input >> vn;
	vector<VertexPositionNormal> verts(vn);
	int index;
	for (auto i = 0; i < vn; ++i)
	{
		input >> index;
		input >> verts[i].normal.x >> verts[i].normal.y >> verts[i].normal.z;
		verts[i].position = positions[index];
	}

	input >> in;
	vector<unsigned short> inds(3 * in);
	vector<Face> faces(in);
	for (auto i = 0; i < in; ++i) {
		input >> inds[3 * i] >> inds[3 * i + 1] >> inds[3 * i + 2];
		faces[i].Indices[0] = inds[3 * i];
		faces[i].Indices[1] = inds[3 * i + 1];
		faces[i].Indices[2] = inds[3 * i + 2];
	}

    map<DirectX::XMFLOAT3, uint, CompareVectors> m_posMap;
    vector<Face> m_uniqueFaces;
    map<Edge, Neighbors, CompareEdges> m_indexMap;
    vector<unsigned short> adjencyIndexes;

    // Step 1 - find the two triangles that share every edge
    for (uint i = 0; i < faces.size(); i++) {
        const Face& face = faces[i];

        Face Unique;

        // If a position vector is duplicated in the VB we fetch the 
        // index of the first occurrence.
        for (uint j = 0; j < 3; j++) {
            uint Index = face.Indices[j];
            DirectX::XMFLOAT3& v = verts[Index].position;

            if (m_posMap.find(v) == m_posMap.end()) {
                m_posMap[v] = Index;
            }
            else {
                Index = m_posMap[v];
            }

            Unique.Indices[j] = Index;
        }

        m_uniqueFaces.push_back(Unique);

        Edge e1(Unique.Indices[0], Unique.Indices[1]);
        Edge e2(Unique.Indices[1], Unique.Indices[2]);
        Edge e3(Unique.Indices[2], Unique.Indices[0]);

        m_indexMap[e1].AddNeigbor(i);
        m_indexMap[e2].AddNeigbor(i);
        m_indexMap[e3].AddNeigbor(i);
    }

    // Step 2 - build the index buffer with the adjacency info
    for (uint i = 0; i < faces.size(); i++) {
        const Face& face = m_uniqueFaces[i];

        for (uint j = 0; j < 3; j++) {
            Edge e(face.Indices[j], face.Indices[(j + 1) % 3]);
            assert(m_indexMap.find(e) != m_indexMap.end());
            Neighbors n = m_indexMap[e];
            uint OtherTri = n.GetOther(i);

            assert(OtherTri != -1);

            const Face& OtherFace = m_uniqueFaces[OtherTri];
            uint OppositeIndex = OtherFace.GetOppositeIndex(e);

            adjencyIndexes.push_back(face.Indices[j]);
            adjencyIndexes.push_back(OppositeIndex);
        }
    }


	return SimpleTriMesh(device, verts, inds, adjencyIndexes);
}
