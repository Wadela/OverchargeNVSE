#pragma once

//Overcharge
#include "Overcharge.hpp"

//Bethesda
#include <ModelLoader.hpp>
#include <BSValueNode.hpp>
#include <BSPSysSimpleColorModifier.hpp>

namespace Overcharge
{
	constexpr float NI_PI = 3.1415927410125732f;
	constexpr float NI_HALF_PI = 0.5f * NI_PI;
	constexpr float NI_TWO_PI = 2.0f * NI_PI;
	constexpr float DEG_TO_RAD = NI_PI / 180.0f;

	extern NiAVObjectPtr OCTranslate;

	extern std::vector<NiParticleSystemPtr> worldSpaceParticles;

	template<typename T>
	static void TraverseNiNode(NiNode* node, const std::function<void(T*)>& callback)
	{
		static_assert(std::is_base_of<NiAVObject, T>::value);

		if (!node) return;

		for (int i = 0; i < node->m_kChildren.m_usSize; ++i)
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
		{
			geom->RemoveProperty(NiProperty::MATERIAL);
			geom->DetachProperty(matProp);
			geom->AddProperty(newMatProp);
			geom->AttachProperty(newMatProp);
			newMatProp->m_fAlpha = matProp->m_fAlpha;
			newMatProp->m_fEmitMult = matProp->m_fEmitMult;
			newMatProp->m_fShine = matProp->m_fShine;
			newMatProp->m_spec = matProp->m_spec;
			newMatProp->m_emit = color;
			geom->UpdateProperties();
			return;
		}

		if (matProp)
		matProp->m_emit = color;
	}

	static void CreateEmissiveColor(NiGeometryPtr geom, const NiColor& color)
	{
		if (!geom) return;
		auto& matProp = geom->m_kProperties.m_spMaterialProperty;
		auto* newMatProp = NiMaterialProperty::CreateObject();

		if (newMatProp)
		{
			geom->RemoveProperty(NiProperty::MATERIAL);
			geom->DetachProperty(matProp);
			geom->AddProperty(newMatProp);
			geom->AttachProperty(newMatProp);
			newMatProp->m_fAlpha = matProp->m_fAlpha;
			newMatProp->m_fEmitMult = matProp->m_fEmitMult;
			newMatProp->m_fShine = matProp->m_fShine;
			newMatProp->m_spec = matProp->m_spec;
			newMatProp->m_emit = color;

			if (auto& ctlr = matProp->m_spControllers) {
				auto newCtlr = ctlr->Clone()->NiDynamicCast<NiTimeController>();
				newCtlr->SetTarget(newMatProp);
			}
			geom->UpdateProperties();
		}
	}

	static void ApplyFixedRotation(NiAVObjectPtr obj, float percent, bool rotX, bool rotY, bool rotZ, bool isNegative = false)
	{
		if (!obj) return;

		float sign = isNegative ? -1.0f : 1.0f;
		float x = rotX ? sign * NI_PI * percent : 0.0f;
		float y = rotY ? sign * NI_PI * percent : 0.0f;
		float z = rotZ ? sign * NI_PI * percent : 0.0f;

		if (obj->IsNiType<NiNode>())
		{
			TraverseNiNode<NiGeometry>(static_cast<NiNode*>(obj.m_pObject), [&](NiGeometryPtr geom) {
				geom->m_kLocal.m_Rotate.FromEulerAnglesXYZ(x, y, z);
				});
		}
		else obj->m_kLocal.m_Rotate.FromEulerAnglesXYZ(x, y, z);
	}

	static void ApplyFixedSpin(NiAVObjectPtr obj, float percent, float time, bool rotX, bool rotY, bool rotZ, bool isNegative = false)
	{
		if (!obj) return;

		float angle = NI_TWO_PI * percent * time;
		float sign = isNegative ? -1.0f : 1.0f;
		float x = rotX ? angle * sign : 0.0f;
		float y = rotY ? angle * sign : 0.0f;
		float z = rotZ ? angle * sign : 0.0f;

		NiMatrix3 rotDelta;
		rotDelta.FromEulerAnglesXYZ(x, y, z);

		if (obj->IsNiType<NiNode>())
		{
			TraverseNiNode<NiGeometry>(static_cast<NiNode*>(obj.m_pObject), [&](NiGeometryPtr geom) {
				geom->m_kLocal.m_Rotate = rotDelta * geom->m_kLocal.m_Rotate;
				});
		}
		else
		{
			obj->m_kLocal.m_Rotate = rotDelta * obj->m_kLocal.m_Rotate;
		}
	}

	inline void ApplyFlicker(float& currentMultiplier, float startMultiplier, float frameTime, UInt32 ticksPassed, bool negative = false)
	{
		const float phaseDuration = 0.15f;
		const float totalDuration = phaseDuration * 2.0f;
		const float endMultiplier = startMultiplier / 20.0f;
		const float time = frameTime * ticksPassed;

		float from = negative ? endMultiplier : startMultiplier;
		float to = negative ? startMultiplier : endMultiplier;

		if (time >= totalDuration) {
			currentMultiplier = negative ? endMultiplier : startMultiplier;
			return;
		}

		if (time < phaseDuration) {
			float t = time / phaseDuration;
			currentMultiplier = from + (to - from) * t;
		}
		else {
			float t = (time - phaseDuration) / phaseDuration;
			currentMultiplier = to + (from - to) * t;
		}
	}

	//Edit Color Modifiers - For preparing particles to have emissive colors pop out more
	static void PrepColorMod(NiParticleSystemPtr& childParticle)
	{
		if (!childParticle) return;

		NiAlphaPropertyPtr alpha = childParticle->GetAlphaProperty();
		if (!alpha || !alpha->IsDstBlendMode(NiAlphaProperty::AlphaFunction::ALPHA_ONE)) return;

		for (NiPSysModifier* it : childParticle->m_kModifierList)
		{
			if (BSPSysSimpleColorModifier* colorMod = it->NiDynamicCast<BSPSysSimpleColorModifier>())
			{
				NiColorA OGCol = colorMod->kColor2;
				const NiColor emit{ OGCol.r, OGCol.g, OGCol.b };
				SetEmissiveColor(childParticle.m_pObject, emit);

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
	static void PrepVertexColor(NiGeometryPtr& geom)
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
			NiNodePtr node = model->spNode;

			TraverseNiNode<NiParticleSystem>(node, [](NiParticleSystemPtr psys) {
				PrepColorMod(psys);
				});

			TraverseNiNode<NiGeometry>(node, [](NiGeometryPtr geom) {
				PrepVertexColor(geom);
				});

			return node;
		}
		else
		{
			if (filePath && (CaseInsensitiveCmp(filePath, "Characters\\_1stPerson\\Skeleton.nif")))
			{
				NiNodePtr node = model->spNode;
				NiFixedString attachName = "##OCTranslate";
				NiAVObjectPtr OCTrn = node->GetObjectByName(attachName);
				if (!OCTrn)
				{
					NiAVObjectPtr bipTrn = node->GetObjectByName("Bip01 Translate");
					if (bipTrn && bipTrn->m_pkParent) 
					{
						NiNodePtr parent = bipTrn->m_pkParent;
						NiNodePtr toAttach = NiNode::CreateObject();
						toAttach->m_kName = attachName;
						toAttach->AttachChild(bipTrn, 0);
						parent->AttachChild(toAttach, 0);
						parent->DetachChildAlt(bipTrn);
					}
				}
				return node;
			}

			char nameBuffer[256];
			for (auto& it : OCExtraModels)
			{
				if (!filePath || !CaseInsensitiveCmp(filePath, it.targetParent.c_str())) continue;

				NiNode* node = model->spNode;
				if (!node) continue;

				if (it.extraNode.index == INVALID_U32 && it.extraNode.flags & OCXColor) {
					if (NiAVObjectPtr hNode = node->GetObjectByName(it.extraNode.nodeName)) {
						if (NiGeometryPtr geom = hNode->NiDynamicCast<NiGeometry>())
							PrepVertexColor(geom);

						else if (NiNodePtr nNode = hNode->NiDynamicCast<NiNode>())
							TraverseNiNode<NiGeometry>(nNode, [](NiGeometryPtr geom) { PrepVertexColor(geom); });
					}
					continue;
				}

				NiNodePtr extraModel = ModelLoader::GetSingleton()->LoadFile(it.extraNode.nodeName, 0, 1, 0, 0, 0);
				if (!extraModel) continue;

				NiObjectPtr extraObj = extraModel->Clone();
				if (!extraObj) continue;

				NiNodePtr extraNode = extraObj->IsNiNode();
				if (!extraNode) continue;

				std::snprintf(nameBuffer, sizeof(nameBuffer), "%s%u", 
					extraNode->m_kName.c_str(), it.extraNode.index);
				NiFixedString nameString(nameBuffer); extraNode->SetName(nameString);
				
				node->AttachChild(extraNode, 0);
				extraNode->m_kLocal.m_fScale = it.xNodeScale;
				extraNode->m_kLocal.m_Translate = it.xNodeTranslate;
				extraNode->m_kLocal.m_Rotate.FromEulerDegrees(
					it.xNodeRotation.x, it.xNodeRotation.y, it.xNodeRotation.z
				);

				if (it.extraNode.flags & OCXParticle) {
					TraverseNiNode<NiParticleSystem>(extraNode, [&](NiParticleSystemPtr psys) {
						if (auto ctlr = psys->GetControllers(); ctlr) ctlr->Stop();
						});
				}
			}
		}
		return model->spNode;
	}

	static void __fastcall ModelModel(const Model* thisPtr, void* edx, char* modelPath, BSStream* fileStream, bool abAssignShaders, bool abKeepUV) 
	{
		ThisStdCall(0x43ACE0, thisPtr, modelPath, fileStream, abAssignShaders, abKeepUV);

		if (thisPtr && g_OCSettings.iVisualEffects > 0)
		ModelLoaderLoadFile(thisPtr, modelPath); 
	}

	inline void Hook()
	{
		WriteRelCall(0x43AB4C, &ModelModel);
	}

	inline void PostLoad() {
		// Add any extra models
		for (const auto& elem : extraModels)
			definedModels.insert(elem);
	}
} 