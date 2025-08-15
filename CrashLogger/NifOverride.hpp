#pragma once

#include "MainHeader.hpp"

//Bethesda
#include <BSValueNode.hpp>
#include <BSPSysSimpleColorModifier.hpp>

//Gamebryo
#include <NiParticleSystem.hpp>


#include "Overcharge.hpp"

namespace Overcharge
{
	template<typename T>
	static void TraverseNiNode(NiNode* node, const std::function<void(T*)>& callback)
	{
		static_assert(std::is_base_of<NiAVObject, T>::value);

		if (!node) return;

		for (int i = 0; i < node->m_kChildren.m_usSize; i++)
		{
			NiAVObject* child = node->m_kChildren[i].m_pObject;

			if (!child) continue;

			if (child->IsNiType<T>())
			{
				callback(static_cast<T*>(child));
			}
			else if (child->IsNiType<NiNode>())
			{
				TraverseNiNode<T>(static_cast<NiNode*>(child), callback);
			}
		}
	}

	static void SetEmissiveColor(NiAVObjectPtr obj, const NiColor& color, NiMaterialPropertyPtr newMatProp = nullptr)
	{
		if (!obj) return;
		auto geom = obj->NiDynamicCast<NiGeometry>();
		if (!geom) return;

		auto& matProp = geom->m_kProperties.m_spMaterialProperty;
		if (newMatProp)
			matProp = newMatProp;

		if (matProp)
			matProp->m_emit = color;
	}

	//Edit Color Modifiers - For preparing particles to have emissive colors pop out more
	static void PrepColorMod(NiParticleSystem* childParticle)
	{
		if (!childParticle) return;

		NiAlphaPropertyPtr alpha = childParticle->GetAlphaProperty();
		if (!alpha || !alpha->IsDstBlendMode(NiAlphaProperty::AlphaFunction::ALPHA_ONE)) return;

		for (NiPSysModifier* it : childParticle->m_kModifierList)
		{
			if (BSPSysSimpleColorModifier* colorMod = it->NiDynamicCast<BSPSysSimpleColorModifier>())
			{
				NiColorA OGCol = colorMod->kColor2;
				NiColorA grayScale = DesaturateRGBA(OGCol, 1.0f);

				grayScale.a = colorMod->kColor1.a;
				colorMod->kColor1 = grayScale;

				grayScale.a = colorMod->kColor2.a;
				colorMod->kColor2 = grayScale;

				grayScale.a = colorMod->kColor3.a;
				colorMod->kColor3 = grayScale;
			}
		}
	}

	//Edit Vertex Colors - Primarily for preparing meshes to have emissive colors to pop out more 
	static void PrepVertexColor(const NiGeometry* geom)
	{
		if (const NiGeometryDataPtr modelData = geom->m_spModelData; modelData && modelData->m_pkColor) 
		{
			for (int i = 0; i < modelData->m_usVertices; i++)
			{
				NiColorA OGCol = modelData->m_pkColor[i];
				NiColorA grayScale = DesaturateRGBA(OGCol, 1.0f);
				modelData->m_pkColor[i] = grayScale;
			}
		}
	}

	static NiNode* __fastcall ModelLoaderLoadFile(const Model* model, const char* filePath) 
	{
		if (filePath && (definedModels.contains(filePath)))
		{
			NiNode* node = model->spNode;

			TraverseNiNode<NiParticleSystem>(node, [](NiParticleSystem* psys) {
				PrepColorMod(psys);
				});

			TraverseNiNode<NiGeometry>(node, [](NiGeometry* geom) {
				PrepVertexColor(geom);
				});

			return node;
		}
		return model->spNode;
	}

	static void __fastcall ModelModel(const Model* thisPtr, void* edx, char* modelPath, BSStream* fileStream, bool abAssignShaders, bool abKeepUV) 
	{
		ThisStdCall(0x43ACE0, thisPtr, modelPath, fileStream, abAssignShaders, abKeepUV);

		if (thisPtr && g_OCSettings.iEnableVisualEffects > 0)
		ModelLoaderLoadFile(thisPtr, modelPath); 
	}

	inline void Hook()
	{
		WriteRelCall(0x43AB4C, &ModelModel);
	}

	inline void PostLoad() {
		// Add any extra models
		for (const std::string elem : extraModels)
		{
			definedModels.insert(elem); 
		}
	}
} 