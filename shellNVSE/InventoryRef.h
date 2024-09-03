#pragma once
#include "PluginAPI.h"

	struct InventoryRef
	{
		struct Data
		{
			TESForm* type;	// 00
			ExtraContainerChanges::EntryData* entry;	// 04
			ExtraDataList* xData;	// 08
		} data;
		TESObjectREFR* containerRef;	// 0C
		TESObjectREFR* tempRef;		// 10
		UInt8				pad14[24];		// 14
		bool				doValidation;	// 2C
		bool				removed;		// 2D
		UInt8				pad2E[2];		// 2E

		SInt32 GetCount() const { return data.entry->countDelta; }
		ExtraDataList* __fastcall SplitFromStack(SInt32 maxStack = 1);
		//+ Any other functions we need

		typedef TESObjectREFR* (__stdcall* InventoryRefCreateFunc)(TESObjectREFR*, TESForm*, SInt32, ExtraDataList*);
		typedef InventoryRef* (*InventoryRefGetForIDFunc)(UInt32);

		static InventoryRefCreateFunc InventoryRefCreate;
		static InventoryRefCreateFunc InventoryRefCreateEntry;
		static InventoryRefGetForIDFunc InventoryRefGetForID;

		static void InitInventoryRefFunct(const NVSEInterface* nvse);

		static ContChangesEntry* __fastcall GetHotkeyItemEntry(UInt8 index, ExtraDataList** outXData);
		static bool __fastcall ClearHotkey(UInt8 index);

	};
