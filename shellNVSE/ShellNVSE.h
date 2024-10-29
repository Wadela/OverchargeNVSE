#pragma once

#include <atomic>
#include <iostream>
#include <unordered_set>

#include "ParamInfos.h"

#include "PluginAPI.h"
#include "AuxVars.h"
#include "PluginFunctions.h"
#include "InventoryRef.h"
#include "GameUI.h" 
#include "common/ICriticalSection.h"
#include "GameData.h"
#include "Hooks_DirectInput8Create.h"
#include "NiNodes.h"

using namespace std::literals;

extern ICriticalSection g_Lock;
extern std::atomic<bool> g_ShowFuncDebug;

extern std::unordered_set<UInt32> DynamicallyCreatedForms;

//NVSE Globals
extern bool (*ExtractArgsEx)(COMMAND_ARGS_EX, ...);
extern bool (*ExtractFormatStringArgs)(UInt32 fmtStringPos, char* buffer, COMMAND_ARGS_EX, UInt32 maxParams, ...);  // From JIP_NVSE.H
extern NVSEArrayVarInterface* g_arrInterface;
extern DIHookControl* g_keyInterface;
extern NVSEArrayVar* (*CreateArray)(const NVSEArrayElement* data, UInt32 size, Script* callingScript);
extern NVSEArrayVar* (*CreateStringMap)(const char** keys, const NVSEArrayElement* values, UInt32 size, Script* callingScript);
extern NVSEArrayVar* (*CreateMap)(const double* keys, const NVSEArrayElement* values, UInt32 size, Script* callingScript);
extern bool (*AssignArrayResult)(NVSEArrayVar* arr, double* dest);
extern void (*SetElement)(NVSEArrayVar* arr, const NVSEArrayElement& key, const NVSEArrayElement& value);
extern void (*AppendElement)(NVSEArrayVar* arr, const NVSEArrayElement& value);
extern UInt32(*GetArraySize)(NVSEArrayVar* arr);
extern NVSEArrayVar* (*LookupArrayByID)(UInt32 id);
extern bool (*GetElement)(NVSEArrayVar* arr, const NVSEArrayElement& key, NVSEArrayElement& outElement);
extern bool (*GetArrayElements)(NVSEArrayVar* arr, NVSEArrayElement* elements, NVSEArrayElement* keys);
extern NVSEStringVarInterface g_strInterface;
extern bool (*AssignString)(COMMAND_ARGS, const char* newValue);
extern const char* (*GetStringVar)(UInt32 stringID);
extern NVSEMessagingInterface* g_msg;
extern NVSEScriptInterface* g_scriptInterface;
extern NiCamera* g_mainCamera;

extern NVSECommandTableInterface g_cmdTableInterface;

extern const CommandInfo* (*GetCmdByName)(const char* name);

extern bool (*FunctionCallScript)(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container, NVSEArrayElement* result, UInt8 numArgs, ...);
extern bool (*FunctionCallScriptAlt)(Script* funcScript, TESObjectREFR* callingObj, UInt8 numArgs, ...);
extern Script* (*pCompileScript)(const char* scriptText);
extern Script* (*pCompileExpression)(const char* scriptText);

template <class T>
T LookupEditorID(const char* edid) {
	return ((T (__cdecl*)(const char*))(0x483A00))(edid); //LookupEditorID
}

extern NVSEEventManagerInterface* g_eventInterface;

//Singletons
extern TileMenu** g_tileMenuArray;
extern UInt32 g_screenWidth;
extern UInt32 g_screenHeight;
extern ActorValueOwner* g_playerAVOwner;
extern InterfaceManager* g_interfaceManager;
extern DataHandler* g_dataHandler;
extern TESObjectWEAP* g_fistsWeapon;

struct ArrayData
{
	UInt32			size;
	std::unique_ptr<ArrayElementR[]> vals;
	std::unique_ptr<ArrayElementR[]> keys;

	ArrayData(NVSEArrayVar* srcArr, bool isPacked)
	{
		size = GetArraySize(srcArr);
		if (size)
		{
			vals = std::make_unique<ArrayElementR[]>(size);
			keys = isPacked ? nullptr : std::make_unique<ArrayElementR[]>(size);
			if (!GetArrayElements(srcArr, vals.get(), keys.get()))
				size = 0;
		}
	}
	ArrayData(NVSEArrayVar* srcArr, NVSEArrayVarInterface::ContainerTypes type) : ArrayData(
		srcArr, type == NVSEArrayVarInterface::ContainerTypes::kArrType_Array)
	{
	}
	~ArrayData() = default;
};
