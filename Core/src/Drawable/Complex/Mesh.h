#pragma once
#include "Drawable/DrawableCommon.h"
#include "Bindable/BindableCommon.h"
#include "Vertex.h"
#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Debug/ConditionalNoexcept.h"


class ModelException : public DXException
{
	public:
		ModelException(int line, const char* file, std::string note) noexcept;
		const char*        what() const noexcept override;
		const char*        GetType() const noexcept override;
		const std::string& GetNote() const noexcept;
	private:
		std::string note_;
};

class Mesh : public DrawableCommon<Mesh>
{
	public:
		Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs);
		void              Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
		DirectX::XMMATRIX GetTransformXM() const noexcept override;
	private:
		mutable DirectX::XMFLOAT4X4 finalTransform_;
};

class Node
{
	friend class Model;
	friend class ModelWindow;
	public:
		Node(const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform) noxnd;
		void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
		void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
	private:
		void AddChild(std::unique_ptr<Node> pChild) noxnd;
		void ShowTree(int& nodeIndex, std::optional<int>& selectedIndex, Node*& pSelectedNode) const noexcept;
	private:
		std::string                        name_;
		std::vector<std::unique_ptr<Node>> childPtrs_;
		// Individual node only retains reference to the meshes via raw pointers
		std::vector<Mesh*> meshPtrs_;
		// transform relative to its parent
		DirectX::XMFLOAT4X4 transform_;
		// transform from the GUI window
		DirectX::XMFLOAT4X4 appliedTransform_;
};

class Model
{
	public:
		Model(Graphics& gfx, const std::string fileName);
		void Draw(Graphics& gfx) const noxnd;
		void ShowWindow(const char* windowName = nullptr) noexcept;
		// we must define this destructor in .cpp file o.w. we won't be able to declare unique_ptr to a forward declared ModelWindow
		~Model() noxnd;
	private:
		static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh);
		std::unique_ptr<Node>        ParseNode(const aiNode& node) noexcept;
	private:
		// we only need to store the root pointer, which will lead us to the rest of the nodes
		std::unique_ptr<Node> pRoot_;
		// Model owns the meshes (thus, we use unique pointers here)
		std::vector<std::unique_ptr<Mesh>> meshPtrs_;
		std::unique_ptr<class ModelWindow> pWindow_;
};
