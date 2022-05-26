#include "AssTest.h"
#include "Bindable/BindableCommon.h"
#include "Debug/GraphicsThrowMacros.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Vertex.h"

AssTest::AssTest(Graphics&                              gfx,
                 std::mt19937&                          rng,
                 std::uniform_real_distribution<float>& adist,
                 std::uniform_real_distribution<float>& ddist,
                 std::uniform_real_distribution<float>& odist,
                 std::uniform_real_distribution<float>& rdist,
                 DirectX::XMFLOAT3                      material,
                 // we can scale the model:
                 float scale)
	:
	TestObject(gfx, rng, adist, ddist, odist, rdist)
{
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{
		// Ultimate goal is to have one source of truth for the vertex layout, which is the shader file
		using D3DEngine::VertexLayout;
		D3DEngine::VertexBuffer vbuf(std::move(
		                                       VertexLayout{}
		                                       .Append(VertexLayout::Position3D)
		                                       .Append(VertexLayout::Normal)
		                                      ));

		// Assimp workflow: 
		Assimp::Importer imp;
		const auto       pModel = imp.ReadFile("Models/suzanne.obj",
		                                       aiProcess_Triangulate |
		                                       // index drawing: 
		                                       aiProcess_JoinIdenticalVertices
		                                      );
		// assume the first mesh is what we want
		const auto pMesh = pModel->mMeshes[0];

		for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
		{
			vbuf.EmplaceBack(
			                 dx::XMFLOAT3
			                 {
				                 // control the size of the model through scale
				                 pMesh->mVertices[i].x * scale,
				                 pMesh->mVertices[i].y * scale,
				                 pMesh->mVertices[i].z * scale
			                 },
			                 *reinterpret_cast<dx::XMFLOAT3*>(&pMesh->mNormals[i])
			                );
		}

		std::vector<unsigned short> indices;
		indices.reserve(pMesh->mNumFaces * 3);
		for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
		{
			const auto& face = pMesh->mFaces[i];
			// assume we have triangulated the mesh
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, vbuf));

		AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, indices));

		auto pvs   = std::make_unique<VertexShader>(gfx, L"Shaders/cso/PhongVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"Shaders/cso/PhongPS.cso"));

		AddStaticBind(std::make_unique<InputLayout>(gfx, vbuf.GetLayout().GetD3DLayout(), pvsbc));

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		struct PSMaterialConstant
		{
			DirectX::XMFLOAT3 color;
			float             specularIntensity = 0.6f;
			float             specularPower     = 30.0f;
			float             padding[3];
		}                     pmc;
		pmc.color = material;
		AddStaticBind(std::make_unique<PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc, 1u));
	}
	else
	{
		SetIndexFromStatic();
	}

	AddBind(std::make_unique<TransformCbuf>(gfx, *this));
}
