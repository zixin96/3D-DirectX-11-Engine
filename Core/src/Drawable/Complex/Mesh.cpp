#include "Mesh.h"
#include "imgui/imgui.h"
#include "Utils/Surface.h"

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
Mesh::Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs)
{
	if (!IsStaticInitialized())
	{
		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	for (auto& pb : bindPtrs)
	{
		if (auto pi = dynamic_cast<IndexBuffer*>(pb.get()))
		{
			AddIndexBuffer(std::unique_ptr<IndexBuffer>{pi});
			// we only want one unique pointer to own the resource,
			// thus we need to release the one in the vector
			pb.release();
		}
		else
		{
			AddBind(std::move(pb));
		}
	}

	AddBind(std::make_unique<TransformCbuf>(gfx, *this));
}

// accumulatedTransform: we need to apply accumulated transform along the tree to the mesh
void Mesh::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd
{
	DirectX::XMStoreFloat4x4(&finalTransform_, accumulatedTransform);
	Drawable::Draw(gfx);
}

DirectX::XMMATRIX Mesh::GetTransformXM() const noexcept
{
	return DirectX::XMLoadFloat4x4(&finalTransform_);
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
	dx::XMStoreFloat4x4(&transform_, transform_in);
	dx::XMStoreFloat4x4(&appliedTransform_, dx::XMMatrixIdentity());
}

// go recursively down the tree to draw the meshes
void Node::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd
{
	const auto built =
			// first apply GUI transform (in object space)
			dx::XMLoadFloat4x4(&appliedTransform_) *
			// transform relative to the parent
			dx::XMLoadFloat4x4(&transform_) *
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
	dx::XMStoreFloat4x4(&appliedTransform_, transform);
}


// pImpl idiom, only defined in this .cpp
// this class is visible only in this .cpp file
class ModelWindow
{
	public:
		void Show(const char* windowName, const Node& root) noexcept
		{
			// window name defaults to "Model"
			windowName = windowName ? windowName : "Model";
			// need an ints to track node indices
			int nodeIndexTracker = 0;
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
					// access selected node transformation
					// if the current index doesn't exist in the map, this will create a new one
					auto& transform = transforms_[pSelectedNode_->GetId()];
					ImGui::Text("Orientation");
					ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f);
					ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f);
					ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f);
					ImGui::Text("Position");
					ImGui::SliderFloat("X", &transform.x, -20.0f, 20.0f);
					ImGui::SliderFloat("Y", &transform.y, -20.0f, 20.0f);
					ImGui::SliderFloat("Z", &transform.z, -20.0f, 20.0f);
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

		// map indices to transform parameters
		// this keeps track of each node's transformation
		std::unordered_map<int, TransformParameters> transforms_;
};

Model::Model(Graphics& gfx, const std::string fileName)
	:
	pWindow_(std::make_unique<ModelWindow>())
{
	Assimp::Importer imp;
	const auto       pScene = imp.ReadFile(fileName.c_str(),
	                                       aiProcess_Triangulate |
	                                       aiProcess_JoinIdenticalVertices |
	                                       aiProcess_ConvertToLeftHanded |
	                                       aiProcess_GenNormals
	                                      );

	if (pScene == nullptr)
	{
		throw ModelException(__LINE__, __FILE__, imp.GetErrorString());
	}

	for (size_t i = 0; i < pScene->mNumMeshes; i++)
	{
		meshPtrs_.push_back(ParseMesh(gfx, *pScene->mMeshes[i], pScene->mMaterials));
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
void Model::ShowWindow(const char* windowName) noexcept
{
	pWindow_->Show(windowName, *pRoot_);
}

Model::~Model() noxnd
{
}

std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials)
{
	using D3DEngine::VertexLayout;

	D3DEngine::VertexBuffer vbuf(std::move(
	                                       VertexLayout{}
	                                       .Append(VertexLayout::Position3D)
	                                       .Append(VertexLayout::Normal)
	                                       .Append(VertexLayout::Texture2D)
	                                      ));

	for (unsigned int i = 0; i < mesh.mNumVertices; i++)
	{
		vbuf.EmplaceBack(
		                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
		                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
		                 *reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i]) // it's possible for a vertex to have separate uv coords for different textures, but usually the textures line up so that only 1 coord is needed to lookup into all of them 
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

	std::vector<std::unique_ptr<Bindable>> bindablePtrs;

	bool hasSpecularMap = false;
	// set a default shininess 
	float shininess = 35.0f;
	// not every mesh has materials 
	if (mesh.mMaterialIndex >= 0)
	{
		auto& material = *pMaterials[mesh.mMaterialIndex];
		using namespace std::string_literals;
		const auto base = "Models/nano_textured/"s;
		aiString   texFileName;
		material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName);
		bindablePtrs.push_back(std::make_unique<Texture>(gfx, Surface::FromFile(base + texFileName.C_Str()), 0));

		// not all meshes have specular map
		if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
		{
			bindablePtrs.push_back(std::make_unique<Texture>(gfx, Surface::FromFile(base + texFileName.C_Str()), 1));
			hasSpecularMap = true;
		}
		else
		{
			// if we don't have specular map, look for constant specified in the file
			// AI_MATKEY_SHININESS expands to 3 arguments.
			material.Get(AI_MATKEY_SHININESS, shininess);
		}

		bindablePtrs.push_back(std::make_unique<Sampler>(gfx));
	}

	bindablePtrs.push_back(std::make_unique<VertexBuffer>(gfx, vbuf));

	bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, indices));


	auto pvs   = std::make_unique<VertexShader>(gfx, L"Shaders/cso/PhongAssVS.cso");
	auto pvsbc = pvs->GetBytecode();
	bindablePtrs.push_back(std::move(pvs));


	if (hasSpecularMap)
	{
		bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"Shaders/cso/PhongPSSpecMap.cso"));
	}
	else
	{
		bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"Shaders/cso/PhongAssPS.cso"));
		struct PSMaterialConstant
		{
			float specularIntensity = 0.8f;
			float specularPower;
			float padding[2];
		}         pmc;
		pmc.specularPower = shininess;
		bindablePtrs.push_back(std::make_unique<PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc, 1u));
	}


	bindablePtrs.push_back(std::make_unique<InputLayout>(gfx, vbuf.GetLayout().GetD3DLayout(), pvsbc));
	return std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
}

std::unique_ptr<Node> Model::ParseNode(int& nextId, const aiNode& node) noexcept
{
	namespace dx = DirectX;
	// parse this node
	const auto transform =
			dx::XMMatrixTranspose(dx::XMLoadFloat4x4(reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)));

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
