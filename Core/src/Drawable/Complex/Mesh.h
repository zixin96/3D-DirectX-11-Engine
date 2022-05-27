#pragma once
#include "Drawable/Drawable.h"
#include "Bindable/BindableCommon.h"
#include "VertexView.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Debug/ConditionalNoexcept.h"

namespace D3DEngine
{
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

	class Mesh : public Drawable
	{
		public:
			Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bindable>> bindPtrs);
			void              Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
			DirectX::XMMATRIX GetTransformXM() const noexcept override;
		private:
			mutable DirectX::XMFLOAT4X4 finalTransform_;
	};

	class Node
	{
		friend class Model;
		public:
			Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform) noxnd;
			void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
			void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
			int  GetId() const noexcept;
			void ShowTree(Node*& pSelectedNode) const noexcept;
		private:
			void AddChild(std::unique_ptr<Node> pChild) noxnd;

			std::string                        name_;
			int                                id_;               // each node has a unique ID to identify them in the tree
			std::vector<std::unique_ptr<Node>> childPtrs_;        // Node owns a set of children node
			std::vector<Mesh*>                 meshPtrs_;         // Individual node only retains reference to the meshes via raw pointers
			DirectX::XMFLOAT4X4                transform_;        // transform relative to its parent
			DirectX::XMFLOAT4X4                appliedTransform_; // transform from the GUI window
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
			static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials);
			std::unique_ptr<Node>        ParseNode(int& nextId, const aiNode& node) noexcept;
		private:
			std::unique_ptr<Node>              pRoot_;    // we only need to store the root pointer, which will lead us to the rest of the nodes
			std::vector<std::unique_ptr<Mesh>> meshPtrs_; // Model owns the meshes (thus, we use unique pointers here)
			std::unique_ptr<class ModelWindow> pWindow_;
	};
}
