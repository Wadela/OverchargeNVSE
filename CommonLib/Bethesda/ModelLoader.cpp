#include "ModelLoader.hpp"

ModelLoader* ModelLoader::GetSingleton() {
	return *(ModelLoader**)0x011C3B3C;
}

NiNode* ModelLoader::LoadFile(const char* apPath, uint32_t aeLODFadeMult, bool abAssignShaders, int unused, bool abKeepUV, bool abNoUseCountIncrease) {
	return ThisStdCall<NiNode*>(0x447080, this, apPath, aeLODFadeMult, abAssignShaders, unused, abKeepUV, abNoUseCountIncrease);
}