#include "NiHeatController.hpp"

#include "NiBinaryStream.hpp"
#include "NiMaterialProperty.hpp"
#include "NiShadeProperty.hpp"
#include "SafeWrite.hpp"
#include "NiStream.hpp"
#include "NiLight.hpp"

#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>

#include "BSPSysSimpleColorModifier.hpp"
#include "BSShaderProperty.hpp"
#include "NiGeometry.hpp"
#include "NiNode.hpp"
#include "NiParticleSystem.hpp"
#include "TESMain.hpp"
#include "NiDX9Renderer.hpp"

NiColor rgbShift(const double time, const double period) 
{
	// Use sine wave functions to smoothly vary RGB values over time
	const float r = static_cast<float>((sin(time / period) + 1) / 2);
	const float g = static_cast<float>((sin((time / period) + 2 * std::numbers::pi / 3) + 1) / 2);
	const float b = static_cast<float>((sin((time / period) + 4 * std::numbers::pi / 3) + 1) / 2);

	// Return the RGB color as a NiColor struct
	return { r, g, b };
}

NiHeatController* __stdcall NiHeatController::CreateObject() 
{
	NiHeatController* pController = NiNew<NiHeatController>();

	// NiTimeController constructor
	ThisStdCall(0xA6D220, pController);

	pController->SetCycleType(LOOP);
	pController->SetAnimType(APP_INIT);
	pController->SetActive(true);

	static void* vtbl_NiHeatController[44] = {};
	if (!vtbl_NiHeatController[0]) 
	{
		for (UInt32 i = 0; i < 44; i++) 
		{
			vtbl_NiHeatController[i] = reinterpret_cast<void***>(pController)[0][i];
		}

		ReplaceVTableEntry(vtbl_NiHeatController, 0,  &NiHeatController::Destroy);
		ReplaceVTableEntry(vtbl_NiHeatController, 2,  &NiHeatController::GetRTTIEx);
		ReplaceVTableEntry(vtbl_NiHeatController, 18, &NiHeatController::CreateCloneEx);
		ReplaceVTableEntry(vtbl_NiHeatController, 19, &NiHeatController::LoadBinaryEx);
		ReplaceVTableEntry(vtbl_NiHeatController, 20, &NiHeatController::LinkObjectEx);
		ReplaceVTableEntry(vtbl_NiHeatController, 35, &NiHeatController::StartEx);
		ReplaceVTableEntry(vtbl_NiHeatController, 36, &NiHeatController::StopEx);
		ReplaceVTableEntry(vtbl_NiHeatController, 37, &NiHeatController::UpdateEx);
	}
	reinterpret_cast<DWORD*>(pController)[0] = reinterpret_cast<DWORD>(vtbl_NiHeatController);

	return pController;
}

void NiHeatController::Destroy(bool abDealloc) 
{
	// Free members
	dynamicShift = nullptr;

	// NiTimeController destructor
	ThisStdCall(0xA6D290, this);

	if ((abDealloc & 1) != 0) {
		NiDelete(this, sizeof(NiHeatController));
	}
}

NiHeatController* NiHeatController::CreateCloneEx(NiCloningProcess* apCloning) 
{
	NiHeatController* pNewController = CreateObject();

	// NiTimeController::CopyMembers
	ThisStdCall(0xA6CFE0, this, pNewController, apCloning);

	return pNewController;
}

void NiHeatController::LoadBinaryEx(NiStream* apStream) const 
{
	ThisStdCall(0xA6D5E0, this, apStream);
	apStream->ReadLinkID();
}

void NiHeatController::LinkObjectEx(NiStream* apStream) const 
{
	ThisStdCall(0xA6D500, this, apStream);
}

void NiHeatController::StartEx(const float afTime) 
{
	SetActive(true);
	m_fLastTime = -3.4028235e38f;
	if (m_usFlags.GetBit(1)) {
		m_fStartTime = afTime;
	}
}

void NiHeatController::StopEx() const {}

void NiHeatController::UpdateEx(NiUpdateData& arData) 
{
	if (!m_pkTarget) 
	{
		return;
	}

	// Only update every 100ms
	const auto time = GetTickCount64();
	if (time - fLastUpdate < 100) 
	{
		return;
	}
	fLastUpdate = time;

	const auto newCol = rgbShift(GetTickCount64());

	if (const auto target = m_pkTarget->NiDynamicCast<NiGeometry>()) 
	{
		if (const auto matProp = target->GetMaterialProperty()) 
		{
			matProp->m_emit = matProp->m_emit.Shifted(newCol, 1);
		}

		if (const auto modelData = target->m_spModelData; modelData && modelData->m_pkColor) 
		{
			for (int i = 0; i < modelData->m_usVertices; i++) 
			{
				auto col = modelData->m_pkColor[i].Shifted(newCol, 1);
				col.a = modelData->m_pkColor[i].a;

				modelData->m_pkColor[i] = col;
			}


			NiDX9Renderer::GetSingleton()->LockPrecacheCriticalSection();
			NiDX9Renderer::GetSingleton()->PurgeGeometryData(modelData);
			NiDX9Renderer::GetSingleton()->UnlockPrecacheCriticalSection();
		}
	}

	if (const auto psys = m_pkTarget->NiDynamicCast<NiParticleSystem>()) 
	{
		for (const auto& mod : psys->m_kModifierList) {
			if (const auto colorMod = mod->NiDynamicCast<BSPSysSimpleColorModifier>()) 
			{
				colorMod->kColor2 = colorMod->kColor2.Shifted(newCol, 1);
			}
		}
	}
}