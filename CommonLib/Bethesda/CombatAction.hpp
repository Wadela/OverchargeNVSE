#pragma once

#include "BSSimpleList.hpp"
#include "CombatWorldStateList.hpp"

class CombatController;
class CombatPlanState;

class CombatAction {
public:
	virtual bool	IsApplicable(CombatController* apController, const CombatPlanState& arState, uint32_t auiValue);
	virtual void	AddProcedureToController(CombatController* apController, int32_t aiKey);
	virtual bool	Unk_02(CombatController* apController, int32_t aiKey);
	virtual float	GetCost(CombatController* apController, uint32_t auiValue) const;
	virtual void	GetName(char* apBuffer, uint32_t auiBufferSize, int32_t aiKey) const;

	struct Descriptor {
		UInt32		uiDefaultFlags;
		const char* pName;
		float		fCost;
	};

	CombatWorldStateList	kBlocks;
	CombatWorldStateList	kRequirements;
	CombatWorldStateList	kTargets;
	CombatWorldStateList	kList1C; // Alternative end states?
	uint32_t				eType;
	Bitfield32				uiFlags;

	bool GetFlag(uint32_t auiFlag) const;
};

ASSERT_SIZE(CombatAction, 0x2C);