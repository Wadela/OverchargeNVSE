#pragma once
#include <memory>
#include <numbers>

#include "NiColor.hpp"
#include "NiTimeController.hpp"

struct HeatData {};

// Function to calculate RGB values based on time
NiColor rgbShift(const double time, const double period = 1000);

class NiHeatController : public NiTimeController {
public:
	bool bUseStaticShift = false;
	NiColor staticShift;
	std::shared_ptr<NiColor> dynamicShift;
	uint32_t fLastUpdate;

	NiHeatController();
	virtual ~NiHeatController();

	static NiHeatController* __stdcall CreateObject();
	void Destroy(bool abDealloc);
	NiHeatController* CreateCloneEx(NiCloningProcess* apCloning);
	void LoadBinaryEx(NiStream* apStream) const;
	void LinkObjectEx(NiStream* apStream) const;
	void StartEx(float afTime);
	void StopEx() const;
	void UpdateEx(NiUpdateData& arData);

	static inline const auto ms_RTTI = NiRTTI("NiHeatController", NiTimeController::ms_RTTI);

	const NiRTTI* GetRTTIEx() const {
		return &NiHeatController::ms_RTTI;
	}
};
