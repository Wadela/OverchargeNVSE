#pragma once

#include <nvse/PluginAPI.hpp>
#include "TESObjectLIGH.hpp"

//From Johnny Guitar NVSE https://github.com/WallSoGB/JohnnyGuitarNVSE/blob/77a15a959cb8ed9af5a4b16daeb332cd894766f8/JG/internal/JohnnyExtraData.hpp

class OCLightExtraData : public PluginFormExtraData, public BSMemObject {
public:
	OCLightExtraData();
	virtual ~OCLightExtraData() override;

	TESObjectLIGH*	pLightBase;

	static const NiFixedString& GetName();

	static void __fastcall Initialize(NVSEDataInterface* apNVSEData);

	static void InitName();

	static [[nodiscard]] bool __fastcall Add(TESForm* apForm, OCLightExtraData* apExtraData);

	static [[nodiscard]] OCLightExtraData* __fastcall Add(TESForm* apForm);
};