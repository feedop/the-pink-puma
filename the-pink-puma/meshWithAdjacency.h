#pragma once

#include "mesh.h"


namespace mini
{
	class MeshWithAdjency : public Mesh {
	public:
        MeshWithAdjency() = default;

		MeshWithAdjency(Mesh&& simpleMesh,
			dx_ptr<ID3D11Buffer>&& adjencyIndices,
			unsigned int adjencyIndexCount
		);

        void RenderWithAdjency(const dx_ptr<ID3D11DeviceContext>& context) const;


		static MeshWithAdjency LoadMesh(const DxDevice& device, const std::wstring& meshPath);

        template<typename VertexType>
        static MeshWithAdjency SimpleTriMesh(const DxDevice& device, const std::vector<VertexType> verts, const std::vector<unsigned short> idxs, const std::vector<unsigned short> adjencyIdxs)
        {
            if (idxs.empty() || adjencyIdxs.empty())
                throw std::runtime_error("Mesh must have indices");
            Mesh mesh = Mesh::SimpleTriMesh(device, verts, idxs);

            auto adjencyInices = device.CreateIndexBuffer(adjencyIdxs);
            
            return MeshWithAdjency(std::move(mesh), std::move(adjencyInices), adjencyIdxs.size());
        }

	private:
		dx_ptr<ID3D11Buffer> m_adjencyIndexBuffer;
		unsigned int m_adjencyIndexCount;
	};


    typedef unsigned int uint;


    // Stores an edge by its vertices and force an order between them
    struct Edge
    {
        Edge(uint _a, uint _b)
        {
            assert(_a != _b);

            if (_a < _b)
            {
                a = _a;
                b = _b;
            }
            else
            {
                a = _b;
                b = _a;
            }
        }

        void Print()
        {
            printf("Edge %d %d\n", a, b);
        }

        uint a;
        uint b;
    };

    struct Neighbors
    {
        unsigned int n1;
        unsigned int n2;

        Neighbors()
        {
            n1 = n2 = (uint)-1;
        }

        void AddNeigbor(uint n)
        {
            if (n1 == -1) {
                n1 = n;
            }
            else if (n2 == -1) {
                n2 = n;
            }
            else {
                assert(0);
            }
        }

        uint GetOther(uint me) const
        {
            if (n1 == me) {
                return n2;
            }
            else if (n2 == me) {
                return n1;
            }
            else {
                assert(0);
            }

            return 0;
        }
    };

    struct CompareEdges
    {
        bool operator()(const Edge& Edge1, const Edge& Edge2) const
        {
            if (Edge1.a < Edge2.a) {
                return true;
            }
            else if (Edge1.a == Edge2.a) {
                return (Edge1.b < Edge2.b);
            }
            else {
                return false;
            }
        }
    };


    struct CompareVectors
    {
        bool operator()(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) const
        {
            if (a.x < b.x) {
                return true;
            }
            else if (a.x == b.x) {
                if (a.y < b.y) {
                    return true;
                }
                else if (a.y == b.y) {
                    if (a.z < b.z) {
                        return true;
                    }
                }
            }

            return false;
        }
    };


    struct Face
    {
        static constexpr uint ArraySize = 3;

        uint Indices[ArraySize];

        uint GetOppositeIndex(const Edge& e) const
        {
            for (uint i = 0; i < ArraySize; i++) {
                uint Index = Indices[i];

                if (Index != e.a && Index != e.b) {
                    return Index;
                }
            }

            assert(0);

            return 0;
        }
    };
};
