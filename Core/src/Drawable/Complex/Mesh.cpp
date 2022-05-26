﻿#include "Mesh.h"
#include "imgui/imgui.h"
#include <unordered_map>
#include <sstream>

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
Node::Node(const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform_in) noxnd
	:
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
// nodeIndexTracked: keep track of current unique index so that we can assign each node a unique index
// selectedIndex: current selected node index (it's optional whether or not one of the nodes is selected)
void Node::ShowTree(int& nodeIndexTracked, std::optional<int>& selectedIndex, Node*& pSelectedNode) const noexcept
{
	// nodeIndex serves as the uid for gui tree nodes, incremented throughout recursion
	const int currentNodeIndex = nodeIndexTracked;
	nodeIndexTracked++;

	// build up flags for current node

	// desired behavior:
	// open when you click the arrow
	// current node is selected only if the current node index == selected node index
	// if the current node has no children, it is a leaf node
	const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
	                        | ((currentNodeIndex == selectedIndex.value_or(-1)) ? ImGuiTreeNodeFlags_Selected : 0)
	                        | ((childPtrs_.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);
	// render this node
	// to use an integer as the node identifier, we must cast it to (void*)(intptr_t) 
	const auto expanded = ImGui::TreeNodeEx(
	                                        (void*)(intptr_t)currentNodeIndex, node_flags, name_.c_str()
	                                       );
	// processing for selecting node
	if (ImGui::IsItemClicked())
	{
		selectedIndex = currentNodeIndex;
		pSelectedNode = const_cast<Node*>(this);
	}

	// recursive rendering of open node's children
	if (expanded)
	{
		for (const auto& pChild : childPtrs_)
		{
			pChild->ShowTree(nodeIndexTracked, selectedIndex, pSelectedNode);
		}
		ImGui::TreePop();
	}
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
				root.ShowTree(nodeIndexTracker, selectedIndex_, pSelectedNode_);

				ImGui::NextColumn();
				// the next column contains all the controls
				if (pSelectedNode_ != nullptr)
				{
					// access selected node transformation
					// if the current index doesn't exist in the map, this will create a new one
					auto& transform = transforms_[*selectedIndex_];
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
			const auto& transform = transforms_.at(*selectedIndex_);
			return
					dx::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
					dx::XMMatrixTranslation(transform.x, transform.y, transform.z);
		}

		Node* GetSelectedNode() const noexcept
		{
			return pSelectedNode_;
		}

	private:
		std::optional<int> selectedIndex_;
		Node*              pSelectedNode_;

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
		meshPtrs_.push_back(ParseMesh(gfx, *pScene->mMeshes[i]));
	}

	pRoot_ = ParseNode(*pScene->mRootNode);
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

std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh)
{
	namespace dx = DirectX;
	using D3DEngine::VertexLayout;

	D3DEngine::VertexBuffer vbuf(std::move(
	                                       VertexLayout{}
	                                       .Append(VertexLayout::Position3D)
	                                       .Append(VertexLayout::Normal)
	                                      ));

	for (unsigned int i = 0; i < mesh.mNumVertices; i++)
	{
		vbuf.EmplaceBack(
		                 *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
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

	std::vector<std::unique_ptr<Bindable>> bindablePtrs;

	bindablePtrs.push_back(std::make_unique<VertexBuffer>(gfx, vbuf));

	bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, indices));

	auto pvs   = std::make_unique<VertexShader>(gfx, L"Shaders/cso/PhongVS.cso");
	auto pvsbc = pvs->GetBytecode();
	bindablePtrs.push_back(std::move(pvs));

	bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"Shaders/cso/PhongPS.cso"));

	bindablePtrs.push_back(std::make_unique<InputLayout>(gfx, vbuf.GetLayout().GetD3DLayout(), pvsbc));

	struct PSMaterialConstant
	{
		DirectX::XMFLOAT3 color             = {0.6f, 0.6f, 0.8f};
		float             specularIntensity = 0.6f;
		float             specularPower     = 30.0f;
		float             padding[3];
	}                     pmc;
	bindablePtrs.push_back(std::make_unique<PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc, 1u));

	return std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
}

std::unique_ptr<Node> Model::ParseNode(const aiNode& node) noexcept
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
	auto pNode = std::make_unique<Node>(node.mName.C_Str(), std::move(curMeshPtrs), transform);

	// parse its children nodes
	for (size_t i = 0; i < node.mNumChildren; i++)
	{
		pNode->AddChild(ParseNode(*node.mChildren[i]));
	}

	return pNode;
}
