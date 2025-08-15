#pragma once

#include "BSMemObject.hpp"
#include "Gamebryo/NiNode.hpp"
#include "BSString.hpp"

class TESForm;

class BGSSceneInfo {
public:
	BGSSceneInfo();
	virtual ~BGSSceneInfo();

	struct Data {
		Data();
		~Data();

		float pData[16];
	};


	Data		kSceneData;
	TESForm*	pAssociatedForm;

	void SetAssociatedForm(TESForm* apForm);
	TESForm* GetAssociatedForm() const;

	void SetSceneData(float apData, uint32_t auiIndex);
	float GetSceneData(uint32_t auiIndex) const;

	double GetMsUsage(uint32_t aeIndex);

	void CollectSceneDataFromNode(NiNode* apNode, bool abIncludeAddonNodes, bool abTrackActors);

	static void CollectSceneDataForLoadedArea(float& afOutTNumber, float& afOutMNumber, bool abCollectTextures, bool abIncludeAddonNodes);
	static void BuildMNumberString(float afMnumber, BSString& aOutText, bool abActor);
};

ASSERT_SIZE(BGSSceneInfo, 0x48)