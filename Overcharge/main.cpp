#include "main.hpp"
#include <format>
#include <iostream>
#include "NifOverride.hpp"
#include <BGSSaveLoadGame.hpp>
#include "OverchargeConfig.hpp"
#include "OverchargeHooks.hpp"
#include "CommandsOvercharge.hpp"
//#include <tracy/Tracy.hpp>

BSSoundHandle* g_SoundHandle;

void InitSingletons()
{
	g_player = PlayerCharacter::GetSingleton();
	g_TESDataHandler = TESDataHandler::GetSingleton();
	g_HUDMainMenu = HUDMainMenu::GetSingleton();
}

void NVSEMessageHandler(NVSEMessagingInterface::Message* msg)
{
	if (msg->type == NVSEMessagingInterface::kMessage_PostLoad)
	{
		Overcharge::Hook();
	}
	if (msg->type == NVSEMessagingInterface::kMessage_DeferredInit)
	{
		InitSingletons();
		Logger::Play();

		for (const auto& i : deferredInit) i(); // call all deferred init functions

		Overcharge::LoadWeaponConfigs("Data\\NVSE\\OCWeapons");
		Overcharge::InitPerks();
		Overcharge::PostLoad();

		OCLightExtraData::InitName();
	}
	else if (msg->type == NVSEMessagingInterface::kMessage_MainGameLoop)
	{
		if (!bMainLoopDoOnce && !MenuMode()) {
			bMainLoopDoOnce = true;
			for (const auto& i : mainLoopDoOnce) i(); // call all do once functions
		}
		for (const auto& i : mainLoop) i(); // call all mainloop functions


		static int OCFrameCounter = 0;
		if (!Overcharge::IsGamePaused() 
		&& !BGSSaveLoadGame::GetSingleton()->IsLoading())	//While the game is running, as long as the game isn't paused or loading 
		{
			Overcharge::UpdatePlayerOCWeapons();										//Cooldown system runs in gameloop
			if (++OCFrameCounter >= Overcharge::NPC_UPDATE_THROTTLE) {
				Overcharge::UpdateActiveOCWeapons();
				OCFrameCounter = 0;
			}
		}
	}
	else if (msg->type == NVSEMessagingInterface::kMessage_PostLoadGame)
	{
		Overcharge::ClearOCWeapons();
	}
}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	g_currentGame = kFalloutNewVegas;

	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Overcharge";
	info->version = 100;

	// version checks
	if (nvse->isEditor) {
		if (nvse->editorVersion < CS_VERSION_1_4_0_518)
		{
			Log() << std::format("Incorrect editor version (got {:08X} need at least {:08X})", nvse->editorVersion, CS_VERSION_1_4_0_518);
			return false;
		}
	} else {
		if (nvse->version < PACKED_NVSE_VERSION) {
			Log() << std::format("NVSE version too old (got {:08X} expected at least {:08X}). Plugin will NOT load! Install the latest version here: https://github.com/xNVSE/NVSE/releases/", nvse->version, PACKED_NVSE_VERSION);
			return false;
		}
		if (nvse->gameVersion < RUNTIME_VERSION_1_4_0_525) {
			Log() << std::format("Incorrect runtime version (got {:08X} need at least {:08X})", nvse->gameVersion, RUNTIME_VERSION_1_4_0_525);
			return false;
		}
		if (nvse->isNogore) {
			Log() << "NoGore is not supported";
			return false;
		}
	}
	return true;
}

bool NVSEPlugin_Load(const NVSEInterface* nvse)
{
	g_pluginHandle = nvse->GetPluginHandle();
	g_nvseInterface = (NVSEInterface*)nvse;
	nvse->SetOpcodeBase(0x4040);

	REG_CMD(CreateOCLightClone);
	REG_CMD(GetOCWeaponState);
	REG_CMD(SetOCWeaponState);
	REG_CMD(GetOCWeaponConfig);
	REG_CMD(GetOCSettings);

	if (nvse->isEditor) {
		return true;
	}


	g_pluginHandle = nvse->GetPluginHandle();
	g_nvseInterface = (NVSEInterface*)nvse;
	g_messagingInterface = static_cast<NVSEMessagingInterface*>(nvse->QueryInterface(NVSEInterface::kInterface_Messaging));
	g_messagingInterface->RegisterListener(g_pluginHandle, "NVSE", NVSEMessageHandler);
	g_scriptInterface = static_cast<NVSEScriptInterface*>(nvse->QueryInterface(NVSEInterface::kInterface_Script));
	ExtractArgsEx = g_scriptInterface->ExtractArgsEx;
	ExtractFormatStringArgs = g_scriptInterface->ExtractFormatStringArgs;
	CallFunctionAlt = g_scriptInterface->CallFunctionAlt;
	CompileExpression = g_scriptInterface->CompileExpression;
	CompileScript = g_scriptInterface->CompileScript;
	g_arrayInterface = static_cast<NVSEArrayVarInterface*>(nvse->QueryInterface(NVSEInterface::kInterface_ArrayVar));
	g_stringInterface = static_cast<NVSEStringVarInterface*>(nvse->QueryInterface(NVSEInterface::kInterface_StringVar));
	g_consoleInterface = static_cast<NVSEConsoleInterface*>(nvse->QueryInterface(NVSEInterface::kInterface_Console));
	g_commandInterface = static_cast<NVSECommandTableInterface*>(nvse->QueryInterface(NVSEInterface::kInterface_CommandTable));
	GetByOpcode = g_commandInterface->GetByOpcode;
	GetStringVar = g_stringInterface->GetString;
	SetStringVar = g_stringInterface->SetString;
	AssignString = g_stringInterface->Assign;
	g_dataInterface = static_cast<NVSEDataInterface*>(nvse->QueryInterface(NVSEInterface::kInterface_Data));
	HasScriptCommand = reinterpret_cast<_HasScriptCommand>(g_dataInterface->GetFunc(NVSEDataInterface::kNVSEData_HasScriptCommand));
	DecompileScript = reinterpret_cast<_DecompileScript>(g_dataInterface->GetFunc(NVSEDataInterface::kNVSEData_DecompileScript));
	g_DIHook = static_cast<DIHookControl*>(g_dataInterface->GetSingleton(NVSEDataInterface::kNVSEData_DIHookControl));
	g_eventInterface = static_cast<NVSEEventManagerInterface*>(nvse->QueryInterface(NVSEInterface::kInterface_EventManager));
	SetNativeEventHandler = g_eventInterface->SetNativeEventHandler;
	RemoveNativeEventHandler = g_eventInterface->RemoveNativeEventHandler;
	SetHandlerFunctionValue = g_eventInterface->SetNativeHandlerFunctionValue;
	RegisterEvent = g_eventInterface->RegisterEvent;
	DispatchEvent = g_eventInterface->DispatchEvent;
	g_loggingInterface = static_cast<NVSELoggingInterface*>(nvse->QueryInterface(NVSEInterface::kInterface_LoggingInterface));

	OCLightExtraData::Initialize(g_dataInterface);

	Overcharge::InitHooks();
	Overcharge::LoadConfigMain("Data\\NVSE\\Plugins\\Overcharge.ini");

	for (const auto& i : pluginLoad) i(); // call all plugin load functions

	return true;
}