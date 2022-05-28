#pragma once
#include "Drawable/Drawable.h"
#include "Bindable/BindableCommon.h"
#include "VertexView.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Debug/ConditionalNoexcept.h"
#include "imgui/imgui.h"

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
			struct PSMaterialConstantFullmonte
			{ 
				BOOL              normalMapEnabled   = TRUE;
				BOOL              specularMapEnabled = TRUE;
				BOOL              hasGlossMap        = FALSE;
				float             specularPower      = 3.1f;
				DirectX::XMFLOAT3 specularColor      = {0.75f, 0.75f, 0.75f};
				float             specularMapWeight  = 0.671f;
			};

			struct PSMaterialConstantNotex
			{
				DirectX::XMFLOAT4 materialColor = {0.447970f, 0.327254f, 0.176283f, 1.0f};
				DirectX::XMFLOAT4 specularColor = {0.65f, 0.65f, 0.65f, 1.0f};
				float             specularPower = 120.0f;
				float             padding[3];
			};

			Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform) noxnd;
			void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
			void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
			int  GetId() const noexcept;
			void ShowTree(Node*& pSelectedNode) const noexcept;

			template <class T>
			bool ControlMeDaddy(Graphics& gfx, T& c)
			{
				if (meshPtrs_.empty())
				{
					return false;
				}

				if constexpr (std::is_same<T, PSMaterialConstantFullmonte>::value)
				{
					if (auto pcb = meshPtrs_.front()->QueryBindable<PixelConstantBuffer<T>>())
					{
						ImGui::Text("Material");

						bool normalMapEnabled = (bool)c.normalMapEnabled;
						ImGui::Checkbox("Norm Map", &normalMapEnabled);
						c.normalMapEnabled = normalMapEnabled ? TRUE : FALSE;

						bool specularMapEnabled = (bool)c.specularMapEnabled;
						ImGui::Checkbox("Spec Map", &specularMapEnabled);
						c.specularMapEnabled = specularMapEnabled ? TRUE : FALSE;

						bool hasGlossMap = (bool)c.hasGlossMap;
						ImGui::Checkbox("Gloss Alpha", &hasGlossMap);
						c.hasGlossMap = hasGlossMap ? TRUE : FALSE;

						ImGui::SliderFloat("Spec Weight", &c.specularMapWeight, 0.0f, 2.0f);

						ImGui::SliderFloat("Spec Pow", &c.specularPower, 0.0f, 1000.0f, "%f");

						ImGui::ColorPicker3("Spec Color", reinterpret_cast<float*>(&c.specularColor));

						pcb->Update(gfx, c);
						return true;
					}
				}
				else if constexpr (std::is_same<T, PSMaterialConstantNotex>::value)
				{
					if (auto pcb = meshPtrs_.front()->QueryBindable<PixelConstantBuffer<T>>())
					{
						ImGui::Text("Material");

						ImGui::ColorPicker3("Spec Color", reinterpret_cast<float*>(&c.specularColor));

						ImGui::SliderFloat("Spec Pow", &c.specularPower, 0.0f, 1000.0f, "%f");

						ImGui::ColorPicker3("Diff Color", reinterpret_cast<float*>(&c.materialColor));

						pcb->Update(gfx, c);
						return true;
					}
				}
				return false;
			}

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
			void ShowWindow(Graphics& gfx, const char* windowName = nullptr) noexcept;
			void SetRootTransform(DirectX::FXMMATRIX tf) noexcept;
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
