#include "OCLightExtraData.hpp"
#include <atomic>
#include <cassert>

#define DEBUG_PRINTS 1

#if DEBUG_PRINTS
#define DEBUG_MSG(...) PrintLog(__VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

namespace JohnnyExtraDataGlobals {
	NiFixedString strName;
	PluginFormExtraData* (*pfGet)(const TESForm*, const char*) = nullptr;
	bool (*pfAdd)(TESForm*, PluginFormExtraData*) = nullptr;
	void (*pfRemoveByName)(TESForm*, const char*) = nullptr;
	void (*pfRemoveByPtr)(TESForm*, PluginFormExtraData*) = nullptr;
}

OCLightExtraData::OCLightExtraData() : PluginFormExtraData(GetName()) {
	pLightBase = nullptr;
}
OCLightExtraData::~OCLightExtraData() {
	delete pLightBase;
	pLightBase = nullptr;
}
const NiFixedString& OCLightExtraData::GetName() {
	assert(JohnnyExtraDataGlobals::strName.m_kHandle);
	return JohnnyExtraDataGlobals::strName;
}
void __fastcall OCLightExtraData::Initialize(NVSEDataInterface* apNVSEData) {
	JohnnyExtraDataGlobals::pfGet = static_cast<PluginFormExtraData * (*)(const TESForm*, const char*)>(apNVSEData->GetFunc(NVSEDataInterface::kNVSEData_FormExtraDataGet));
	JohnnyExtraDataGlobals::pfAdd = static_cast<bool(*)(TESForm*, PluginFormExtraData*)>(apNVSEData->GetFunc(NVSEDataInterface::kNVSEData_FormExtraDataAdd));
	JohnnyExtraDataGlobals::pfRemoveByName = static_cast<void (*)(TESForm*, const char*)>(apNVSEData->GetFunc(NVSEDataInterface::kNVSEData_FormExtraDataRemoveByName));
	JohnnyExtraDataGlobals::pfRemoveByPtr = static_cast<void (*)(TESForm*, PluginFormExtraData*)>(apNVSEData->GetFunc(NVSEDataInterface::kNVSEData_FormExtraDataRemoveByPtr));
}
void OCLightExtraData::InitName() {
	JohnnyExtraDataGlobals::strName = "OCLightExtraData";
}
bool __fastcall OCLightExtraData::Add(TESForm* apForm, OCLightExtraData* apExtraData) {
	if (apForm && apForm->eTypeID == TESForm::kType_TESObjectLIGH 
		&& JohnnyExtraDataGlobals::pfAdd(apForm, apExtraData)) {
		apExtraData->pLightBase = reinterpret_cast<TESObjectLIGH*>(apForm);
		return true;
	}
	return false;
}
OCLightExtraData* __fastcall OCLightExtraData::Add(TESForm* apForm) {
	OCLightExtraData* pExtraData = new OCLightExtraData();
	if (Add(apForm, pExtraData))
		return pExtraData;
	delete pExtraData;
	return nullptr;
}