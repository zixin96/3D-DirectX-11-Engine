#include "Mesh.h"
#include "imgui/imgui.h"
#include "Utils/D3DXM.h"
#include "Utils/Surface.h"
#include <filesystem>

namespace D3DEngine
{
	namespace dx = DirectX;

	ModelException::ModelException(int line, const char* file, std::string note) noexcept
		:
		DXException(line, file),
		note_(std::move(note))
	{
	}

	const char* ModelException::what() const noexcept
	{
		std::ostringstream oss;
		oss << DXException::what() << std::endl
				<< "[Note] " << GetNote();
		whatBuffer_ = oss.str();
		return whatBuffer_.c_str();
	}

	const char* ModelException::GetType() const noexcept
	{
		return "3D Game Programming Model Exception";
	}

	const std::string& ModelException::GetNote() const noexcept
	{
		return note_;
	}

	// ---------------------------------------------------------------------------

	// by passing in a vector of bindables, we want to the user to decide which bindable this mesh has
	Mesh::Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bindable>> bindPtrs)
	{
		AddBind(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		for (auto& pb : bindPtrs)
		{
			AddBind(std::move(pb));
		}

		AddBind(std::make_shared<TransformCbuf>(gfx, *this));
	}

	// accumulatedTransform: we need to apply accumulated transform along the tree to the mesh
	void Mesh::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd
	{
		XMStoreFloat4x4(&finalTransform_, accumulatedTransform);
		Drawable::Draw(gfx);
	}

	DirectX::XMMATRIX Mesh::GetTransformXM() const noexcept
	{
		return XMLoadFloat4x4(&finalTransform_);
	}

	// ---------------------------------------------------------------------------

	// each node has its own name (for identifying it in the tree), a set of meshes, and its transform
	// Node
	Node::Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform_in) noxnd
		:
		id_(id),
		meshPtrs_(std::move(meshPtrs)),
		name_(name)
	{
		XMStoreFloat4x4(&transform_, transform_in);
		XMStoreFloat4x4(&appliedTransform_, dx::XMMatrixIdentity());
	}

	// go recursively down the tree to draw the meshes
	void Node::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd
	{
		const auto built =
				// first apply GUI transform (in object space)
				XMLoadFloat4x4(&appliedTransform_) *
				// transform relative to the parent
				XMLoadFloat4x4(&transform_) *
				// transform of all the ancestors
				accumulatedTransform;

		for (const auto pMesh : meshPtrs_)
		{
			pMesh->Draw(gfx, built);
		}
		for (const auto& pChildNode : childPtrs_)
		{
			pChildNode->Draw(gfx, built);
		}
	}

	void Node::AddChild(std::unique_ptr<Node> pChild) noxnd
	{
		assert(pChild);
		childPtrs_.push_back(std::move(pChild));
	}

	// recursively show nodes in the tree UI
	void Node::ShowTree(Node*& pSelectedNode) const noexcept
	{
		// if there is no selected node, set selectedId to an impossible value
		const int selectedId = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetId();

		// build up flags for current node

		// desired behavior:
		// open when you click the arrow
		// current node is selected only if the current node index == selected node index
		// if the current node has no children, it is a leaf node
		const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
		                        | ((GetId() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0)
		                        | ((childPtrs_.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);
		// render this node
		// to use an integer as the node identifier, we must cast it to (void*)(intptr_t) 
		const auto expanded = ImGui::TreeNodeEx(
		                                        (void*)(intptr_t)GetId(), node_flags, name_.c_str()
		                                       );
		// processing for selecting node
		if (ImGui::IsItemClicked())
		{
			pSelectedNode = const_cast<Node*>(this);
		}

		// recursive rendering of open node's children
		if (expanded)
		{
			for (const auto& pChild : childPtrs_)
			{
				pChild->ShowTree(pSelectedNode);
			}
			ImGui::TreePop();
		}
	}

	int Node::GetId() const noexcept
	{
		return id_;
	}

	void Node::SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept
	{
		XMStoreFloat4x4(&appliedTransform_, transform);
	}

	const DirectX::XMFLOAT4X4& Node::GetAppliedTransform() const noexcept
	{
		return appliedTransform_;
	}

	// pImpl idiom, only defined in this .cpp
	// this class is visible only in this .cpp file
	class ModelWindow
	{
		public:
			void Show(Graphics& gfx, const char* windowName, const Node& root) noexcept
			{
				// window name defaults to "Model"
				windowName = windowName ? windowName : "Model";
				if (ImGui::Begin(windowName))
				{
					// 2 columns with a divider in-between
					ImGui::Columns(2, nullptr, true);
					// start the recursion to generate the tree widget
					root.ShowTree(pSelectedNode_);

					ImGui::NextColumn();
					// the next column contains all the controls
					if (pSelectedNode_ != nullptr)
					{
						const auto id = pSelectedNode_->GetId();
						auto       i  = transforms_.find(id);
						if (i == transforms_.end())
						{
							const auto&         applied     = pSelectedNode_->GetAppliedTransform();
							const auto          angles      = ExtractEulerAngles(applied);
							const auto          translation = ExtractTranslation(applied);
							TransformParameters tp;
							tp.roll                  = angles.z;
							tp.pitch                 = angles.x;
							tp.yaw                   = angles.y;
							tp.x                     = translation.x;
							tp.y                     = translation.y;
							tp.z                     = translation.z;
							std::tie(i, std::ignore) = transforms_.insert({id, tp});
						}
						auto& transform = i->second;

						// access selected node transformation
						// if the current index doesn't exist in the map, this will create a new one
						// auto& transform = transforms_[pSelectedNode_->GetId()];

						ImGui::Text("Orientation");
						ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f);
						ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f);
						ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f);
						ImGui::Text("Position");
						ImGui::SliderFloat("X", &transform.x, -20.0f, 20.0f);
						ImGui::SliderFloat("Y", &transform.y, -20.0f, 20.0f);
						ImGui::SliderFloat("Z", &transform.z, -20.0f, 20.0f);
						if (!pSelectedNode_->ControlMeDaddy(gfx, skinMaterial))
						{
							pSelectedNode_->ControlMeDaddy(gfx, ringMaterial);
						}
					}
				}
				ImGui::End();
			}

			dx::XMMATRIX GetTransform() const noexcept
			{
				assert(pSelectedNode_ != nullptr);
				const auto& transform = transforms_.at(pSelectedNode_->GetId());
				return
						dx::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
						dx::XMMatrixTranslation(transform.x, transform.y, transform.z);
			}

			Node* GetSelectedNode() const noexcept
			{
				return pSelectedNode_;
			}

		private:
			Node* pSelectedNode_;

			struct TransformParameters
			{
				float roll  = 0.0f;
				float pitch = 0.0f;
				float yaw   = 0.0f;
				float x     = 0.0f;
				float y     = 0.0f;
				float z     = 0.0f;
			};

			Node::PSMaterialConstantFullmonte skinMaterial;
			Node::PSMaterialConstantNotex     ringMaterial;
			// map indices to transform parameters
			// this keeps track of each node's transformation
			std::unordered_map<int, TransformParameters> transforms_;
	};

	Model::Model(Graphics& gfx, const std::string& pathString, const float scale)
		:
		pWindow_(std::make_unique<ModelWindow>())
	{
		Assimp::Importer imp;
		const auto       pScene = imp.ReadFile(pathString.c_str(),
		                                       aiProcess_Triangulate |
		                                       aiProcess_JoinIdenticalVertices |
		                                       aiProcess_ConvertToLeftHanded |
		                                       aiProcess_GenNormals |
		                                       aiProcess_CalcTangentSpace
		                                      );

		if (pScene == nullptr)
		{
			throw ModelException(__LINE__, __FILE__, imp.GetErrorString());
		}

		for (size_t i = 0; i < pScene->mNumMeshes; i++)
		{
			meshPtrs_.push_back(ParseMesh(gfx, *pScene->mMeshes[i], pScene->mMaterials, pathString, scale));
		}

		int nextId = 0;
		pRoot_     = ParseNode(nextId, *pScene->mRootNode);
	}

	void Model::Draw(Graphics& gfx) const noxnd
	{
		if (auto node = pWindow_->GetSelectedNode())
		{
			node->SetAppliedTransform(pWindow_->GetTransform());
		}
		// start the recursion down the tree at the root node 
		pRoot_->Draw(gfx, dx::XMMatrixIdentity());
	}

	// show the final tree window using IMGUI
	void Model::ShowWindow(Graphics& gfx, const char* windowName) noexcept
	{
		pWindow_->Show(gfx, windowName, *pRoot_);
	}

	void Model::SetRootTransform(DirectX::FXMMATRIX tf) noexcept
	{
		pRoot_->SetAppliedTransform(tf);
	}

	Model::~Model() noxnd
	{
	}

	std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, float scale)
	{
		using namespace std::string_literals;
		std::vector<std::shared_ptr<Bindable>> bindablePtrs;

		const auto rootPath = path.parent_path().string() + "\\";

		bool         hasSpecularMap = false;
		bool         hasAlphaGloss  = false;
		bool         hasNormalMap   = false;
		bool         hasDiffuseMap  = false;
		float        shininess      = 2.0f;
		dx::XMFLOAT4 specularColor  = {0.18f, 0.18f, 0.18f, 1.0f};
		dx::XMFLOAT4 diffuseColor   = {0.45f, 0.45f, 0.85f, 1.0f};
		if (mesh.mMaterialIndex >= 0)
		{
			auto& material = *pMaterials[mesh.mMaterialIndex];

			aiString texFileName;

			if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
			{
				bindablePtrs.push_back(Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 0));
				hasDiffuseMap = true;
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));
			}

			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
			{
				auto tex      = Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 1);
				hasAlphaGloss = tex->HasAlpha();
				bindablePtrs.push_back(std::move(tex));
				hasSpecularMap = true;
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(specularColor));
			}

			if (!hasAlphaGloss)
			{
				material.Get(AI_MATKEY_SHININESS, shininess);
			}

			if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
			{
				auto tex = Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 2);
				bindablePtrs.push_back(std::move(tex));
				hasNormalMap = true;
			}

			if (hasDiffuseMap || hasSpecularMap || hasNormalMap)
			{
				bindablePtrs.push_back(Sampler::Resolve(gfx));
			}
		}

		// want the full path here
		const auto meshTag = path.string() + "%" + mesh.mName.C_Str();

		if (hasDiffuseMap && hasNormalMap && hasSpecularMap)
		{
			RawVertexBufferWithLayout vbuf(std::move(
			                                         DynamicVertexLayout{}
			                                         .Append(DynamicVertexLayout::Position3D)
			                                         .Append(DynamicVertexLayout::Normal)
			                                         .Append(DynamicVertexLayout::Tangent)
			                                         .Append(DynamicVertexLayout::Bitangent)
			                                         .Append(DynamicVertexLayout::Texture2D)
			                                        ));

			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vbuf.EmplaceBack(
				                 dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]),
				                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]),
				                 *reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
				                );
			}

			std::vector<unsigned short> indices;
			indices.reserve(mesh.mNumFaces * 3);
			for (unsigned int i = 0; i < mesh.mNumFaces; i++)
			{
				const auto& face = mesh.mFaces[i];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}

			bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));

			bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

			auto pvs   = VertexShader::Resolve(gfx, "Shaders/cso/PhongVSNormalMap.cso");
			auto pvsbc = pvs->GetBytecode();
			bindablePtrs.push_back(std::move(pvs));

			bindablePtrs.push_back(PixelShader::Resolve(gfx, "Shaders/cso/PhongPSSpecNormalMap.cso"));

			bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pvsbc));

			Node::PSMaterialConstantFullmonte pmc;
			pmc.specularPower = shininess;
			pmc.hasGlossMap   = hasAlphaGloss ? TRUE : FALSE;
			// this is CLEARLY an issue... all meshes will share same mat const, but may have different
			// Ns (specular power) specified for each in the material properties... bad conflict
			bindablePtrs.push_back(PixelConstantBuffer<Node::PSMaterialConstantFullmonte>::Resolve(gfx, pmc, 1u));
		}
		else if (hasDiffuseMap && hasNormalMap)
		{
			RawVertexBufferWithLayout vbuf(std::move(
			                                         DynamicVertexLayout{}
			                                         .Append(DynamicVertexLayout::Position3D)
			                                         .Append(DynamicVertexLayout::Normal)
			                                         .Append(DynamicVertexLayout::Tangent)
			                                         .Append(DynamicVertexLayout::Bitangent)
			                                         .Append(DynamicVertexLayout::Texture2D)
			                                        ));

			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vbuf.EmplaceBack(
				                 dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]),
				                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]),
				                 *reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
				                );
			}

			std::vector<unsigned short> indices;
			indices.reserve(mesh.mNumFaces * 3);
			for (unsigned int i = 0; i < mesh.mNumFaces; i++)
			{
				const auto& face = mesh.mFaces[i];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}

			bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));

			bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

			auto pvs   = VertexShader::Resolve(gfx, "Shaders/cso/PhongVSNormalMap.cso");
			auto pvsbc = pvs->GetBytecode();
			bindablePtrs.push_back(std::move(pvs));

			bindablePtrs.push_back(PixelShader::Resolve(gfx, "Shaders/cso/PhongPSNormalMap.cso"));

			bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pvsbc));

			struct PSMaterialConstantDiffnorm
			{
				float specularIntensity;
				float specularPower;
				BOOL  normalMapEnabled = TRUE;
				float padding[1];
			}         pmc;
			pmc.specularPower     = shininess;
			pmc.specularIntensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
			// this is CLEARLY an issue... all meshes will share same mat const, but may have different
			// Ns (specular power) specified for each in the material properties... bad conflict
			bindablePtrs.push_back(PixelConstantBuffer<PSMaterialConstantDiffnorm>::Resolve(gfx, pmc, 1u));
		}
		else if (hasDiffuseMap && !hasNormalMap && hasSpecularMap)
		{
			RawVertexBufferWithLayout vbuf(std::move(
			                                         DynamicVertexLayout{}
			                                         .Append(DynamicVertexLayout::Position3D)
			                                         .Append(DynamicVertexLayout::Normal)
			                                         .Append(DynamicVertexLayout::Texture2D)
			                                        ));

			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vbuf.EmplaceBack(
				                 dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				                 *reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
				                );
			}

			std::vector<unsigned short> indices;
			indices.reserve(mesh.mNumFaces * 3);
			for (unsigned int i = 0; i < mesh.mNumFaces; i++)
			{
				const auto& face = mesh.mFaces[i];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}

			bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));

			bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

			auto pvs   = VertexShader::Resolve(gfx, "Shaders/cso/PhongPosNormTexVS.cso");
			auto pvsbc = pvs->GetBytecode();
			bindablePtrs.push_back(std::move(pvs));

			bindablePtrs.push_back(PixelShader::Resolve(gfx, "Shaders/cso/PhongPSSpec.cso"));
			bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pvsbc));

			struct PSMaterialConstantDiffuseSpec
			{
				float specularPowerConst;
				BOOL  hasGloss;
				float specularMapWeight;
				float padding;
			}         pmc;
			pmc.specularPowerConst = shininess;
			pmc.hasGloss           = hasAlphaGloss ? TRUE : FALSE;
			pmc.specularMapWeight  = 1.0f;
			// this is CLEARLY an issue... all meshes will share same mat const, but may have different
			// Ns (specular power) specified for each in the material properties... bad conflict
			bindablePtrs.push_back(PixelConstantBuffer<PSMaterialConstantDiffuseSpec>::Resolve(gfx, pmc, 1u));
		}
		else if (hasDiffuseMap)
		{
			RawVertexBufferWithLayout vbuf(std::move(
			                                         DynamicVertexLayout{}
			                                         .Append(DynamicVertexLayout::Position3D)
			                                         .Append(DynamicVertexLayout::Normal)
			                                         .Append(DynamicVertexLayout::Texture2D)
			                                        ));

			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vbuf.EmplaceBack(
				                 dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				                 *reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
				                );
			}

			std::vector<unsigned short> indices;
			indices.reserve(mesh.mNumFaces * 3);
			for (unsigned int i = 0; i < mesh.mNumFaces; i++)
			{
				const auto& face = mesh.mFaces[i];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}

			bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));

			bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

			auto pvs   = VertexShader::Resolve(gfx, "Shaders/cso/PhongPosNormTexVS.cso");
			auto pvsbc = pvs->GetBytecode();
			bindablePtrs.push_back(std::move(pvs));

			bindablePtrs.push_back(PixelShader::Resolve(gfx, "Shaders/cso/PhongPosNormTexPS.cso"));

			bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pvsbc));

			struct PSMaterialConstantDiffuse
			{
				float specularIntensity;
				float specularPower;
				float padding[2];
			}         pmc;
			pmc.specularPower     = shininess;
			pmc.specularIntensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
			// this is CLEARLY an issue... all meshes will share same mat const, but may have different
			// Ns (specular power) specified for each in the material properties... bad conflict
			bindablePtrs.push_back(PixelConstantBuffer<PSMaterialConstantDiffuse>::Resolve(gfx, pmc, 1u));
		}
		else if (!hasDiffuseMap && !hasNormalMap && !hasSpecularMap)
		{
			RawVertexBufferWithLayout vbuf(std::move(
			                                         DynamicVertexLayout{}
			                                         .Append(DynamicVertexLayout::Position3D)
			                                         .Append(DynamicVertexLayout::Normal)
			                                        ));

			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vbuf.EmplaceBack(
				                 dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i])
				                );
			}

			std::vector<unsigned short> indices;
			indices.reserve(mesh.mNumFaces * 3);
			for (unsigned int i = 0; i < mesh.mNumFaces; i++)
			{
				const auto& face = mesh.mFaces[i];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}

			bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));

			bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

			auto pvs   = VertexShader::Resolve(gfx, "Shaders/cso/PhongVSNotex.cso");
			auto pvsbc = pvs->GetBytecode();
			bindablePtrs.push_back(std::move(pvs));

			bindablePtrs.push_back(PixelShader::Resolve(gfx, "Shaders/cso/PhongPSNotex.cso"));

			bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pvsbc));

			Node::PSMaterialConstantNotex pmc;
			pmc.specularPower = shininess;
			pmc.specularColor = specularColor;
			pmc.materialColor = diffuseColor;
			// this is CLEARLY an issue... all meshes will share same mat const, but may have different
			// Ns (specular power) specified for each in the material properties... bad conflict
			bindablePtrs.push_back(PixelConstantBuffer<Node::PSMaterialConstantNotex>::Resolve(gfx, pmc, 1u));
		}
		else
		{
			throw std::runtime_error("terrible combination of textures in material smh");
		}

		return std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
	}

	std::unique_ptr<Node> Model::ParseNode(int& nextId, const aiNode& node) noexcept
	{
		namespace dx = DirectX;
		// parse this node
		const auto transform = dx::XMMatrixTranspose(dx::XMLoadFloat4x4(
		                                                                reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
		                                                               ));
		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			curMeshPtrs.push_back(meshPtrs_.at(meshIdx).get());
		}
		auto pNode = std::make_unique<Node>(nextId++, node.mName.C_Str(), std::move(curMeshPtrs), transform);

		// parse its children nodes
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			pNode->AddChild(ParseNode(nextId, *node.mChildren[i]));
		}

		return pNode;
	}
}
