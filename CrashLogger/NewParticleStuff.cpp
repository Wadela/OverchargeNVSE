/*struct ProjectileHeatValue {
	NiColorA kColor;
	uint32_t uiRefId;
};

std::unordered_map<NiAVObject*, std::shared_ptr<ProjectileHeatValue>> trackedNodes{};

// Lookups for utility, weak_ptr makes it a little annoying though
std::unordered_map<uint32_t, std::weak_ptr<ProjectileHeatValue>> projectileHeat{};
std::unordered_map<BSPSysSimpleColorModifier*, std::vector<std::weak_ptr<ProjectileHeatValue>>> colorModifiers{};

// Collect all BSValue nodes (game will 'attach' emitters to these)
void FindValueNodes(const NiNode* apNode, std::vector<BSValueNode*>& result) {
	for (int i = 0; i < apNode->m_kChildren.GetSize(); i++) {
		const auto child = apNode->m_kChildren.GetAt(i);

		if (const auto pValueNode = child->NiDynamicCast<BSValueNode>()) {
			result.push_back(pValueNode);
		}

		else if (const auto pNiNode = child->NiDynamicCast<NiNode>()) {
			FindValueNodes(pNiNode, result);
		}
	}
}

std::vector<BSValueNode*> FindValueNodes(const NiNode* apNode) {
	std::vector<BSValueNode*> valueNodes{};
	FindValueNodes(apNode, valueNodes);
	return valueNodes;
}

CallHook<0x9BE07A, CallConv::Cdecl, bool> orig_Projectile_Init3D_9BE07A;
inline bool __cdecl hk_Projectile_Init3D_9BE07A(NiNode* pProjectileRoot) {
	const auto parentFrame = orig_Projectile_Init3D_9BE07A.GetParentFrame();
	const auto pProjectile = parentFrame.Get<Projectile*>(-0x8);

	const auto heatValue = std::make_shared<ProjectileHeatValue>(
		NiColorA(
			(rand() % 256) / 255.0,
			(rand() % 256) / 255.0,
			(rand() % 256) / 255.0,
			1.0
		),
		pProjectile->uiFormID 
	);

	// Mark value nodes to track (for emission later)
	for (const auto& valueNode : FindValueNodes(pProjectileRoot)) {
		trackedNodes[valueNode] = heatValue;
	}

	// Track initial heat for this projectile (later used for impact creation)
	projectileHeat[pProjectile->uiFormID] = heatValue;

	return orig_Projectile_Init3D_9BE07A(pProjectileRoot);
}

std::shared_ptr<ProjectileHeatValue> curHeat{};

CallHook<0x9C2AC3, CallConv::Cdecl, BSTempEffectParticle*> orig_Projectile_SpawnCollisionEffects_9C2AC3;
inline BSTempEffectParticle* __cdecl hk_Projectile_SpawnCollisionEffects_9C2AC3(
	TESObjectCELL* apCell,
	float afLifetime,
	const char* apFilename,
	NiPoint3 a4,
	NiPoint3 a5,
	float a6,
	char a7,
	NiRefObject* apParent) {

	const auto parentFrame = orig_Projectile_SpawnCollisionEffects_9C2AC3.GetParentFrame();
	const auto pProjectile = parentFrame.Get<Projectile*>(-0x2B8);

	// BSTempEffectParticle::Create
	const auto pParticle = orig_Projectile_SpawnCollisionEffects_9C2AC3(apCell, afLifetime, apFilename, a4, a5, a6, a7, apParent);
	if (!pParticle) {
		return nullptr;
	}

	// Harden this
	const auto asNode = static_cast<NiAVObject*>(pParticle->spParticleObject.m_pObject)->NiDynamicCast<NiNode>();
	if (!asNode || !projectileHeat.contains(pProjectile->uiFormID)) {
		return pParticle;
	}

	// Attempt to lock the weak ptr / track new value nodes
	if (const auto shared = projectileHeat[pProjectile->uiFormID].lock()) {
		for (const auto& valueNode : FindValueNodes(asNode)) {
			trackedNodes[valueNode] = shared;
		}
	}

	return pParticle;
}

PrologueHook<0xC31CF0, CallConv::This> orig_NiPSysVolumeEmitter_ComputeInitialPositionAndVelocity;
inline void __fastcall hk_NiPSysVolumeEmitter_ComputeInitialPositionAndVelocity(NiPSysVolumeEmitter* thisPtr, void*, NiPoint3* arg0, NiPoint3* arg4) {
	orig_NiPSysVolumeEmitter_ComputeInitialPositionAndVelocity(thisPtr, arg0, arg4);

	if (trackedNodes.contains(thisPtr->m_pkEmitterObj)) {
		const auto& heatInfo = trackedNodes[thisPtr->m_pkEmitterObj];

		// Game uses this inside of NiPSysEmitter::EmitParticles (0xC220C0) to set the vertex color of the next spawned particle
		thisPtr->m_kInitialColor = heatInfo->kColor;

		// Track (+ hook) BSPSysSimpleColorModifier since it overwrites all vertex colors upon init
		for (const auto modifier : thisPtr->m_pkTarget->m_kModifierList) {
			if (const auto scm = modifier->NiDynamicCast<BSPSysSimpleColorModifier>()) {
				colorModifiers[scm].push_back(heatInfo);
			}
		}

		// Set emission to white for vertex colors to work
		if (const auto matProp = thisPtr->m_pkTarget->GetMaterialProperty()) {
			matProp->m_emit = NiColor::White;
		}
	}
}


PrologueHook<0xC602E0, CallConv::This> orig_BSPSysSimpleColorModifier_Update;
inline void __fastcall hk_BSPSysSimpleColorModifier_Update(BSPSysSimpleColorModifier* apThis, void*, float afTime, NiPSysData* apData) {
	if (colorModifiers.contains(apThis)) {
		if (apData->m_pkColor) {
			// Force BSPSysSimpleColorModifier to only fuck with first vertex
			apData->m_usActiveVertices = 1;

			// Store old data
			const auto oldNumVertices = apData->m_usActiveVertices;
			const auto oldColor = apData->m_pkColor[0];

			orig_BSPSysSimpleColorModifier_Update(apThis, afTime, apData);

			// Modified vertex color
			const auto newColor = apData->m_pkColor[0];

			// Restore old data
			apData->m_usActiveVertices = oldNumVertices;
			apData->m_pkColor[0] = oldColor;

			// Now update alpha of each affected heat controller
			for (const auto& hv : colorModifiers[apThis]) {
				if (const auto sptr = hv.lock()) {
					sptr->kColor.a = newColor.a;
				}
			}

			// Update vertex color alphas
			for (int i = 0; i < apData->m_usActiveVertices; i++) {
				apData->m_pkColor[i].a = newColor.a;
			}
		}

		// Skip vanilla
		return;
	}

	orig_BSPSysSimpleColorModifier_Update(apThis, afTime, apData);
}

InlineHook<0xC5E167> orig_BSMasterParticleSystem_ClearUnreferencedEmitters_C5E167;
inline void hk_BSMasterParticleSystem_ClearUnreferencedEmitters_C5E167() {
	const auto parentFrame = orig_BSMasterParticleSystem_ClearUnreferencedEmitters_C5E167.GetParentFrame();
	const auto toDelete = parentFrame.Get<NiAVObject*>(-0x20);

	// Stop tracking emitters when master particle system drops them
	trackedNodes.erase(toDelete);

	// Clean up weak pointers
	// Yes this is nasty
	{
		std::erase_if(projectileHeat, [](auto& elem) {
			const auto& [_, value] = elem;
			return value.expired();
			});

		std::erase_if(colorModifiers, [](auto& elem) {
			auto& [_, value] = elem;

			// Remove from inner vector
			std::erase_if(value, [](auto& item) {
				return item.expired();
				});

			// Remove from map if vector empty
			return value.empty();
			});
	}
}

void InitHooks() {
	orig_NiPSysVolumeEmitter_ComputeInitialPositionAndVelocity.init(&hk_NiPSysVolumeEmitter_ComputeInitialPositionAndVelocity);
	orig_Projectile_Init3D_9BE07A.init(&hk_Projectile_Init3D_9BE07A);
	orig_Projectile_SpawnCollisionEffects_9C2AC3.init(&hk_Projectile_SpawnCollisionEffects_9C2AC3);
	orig_BSPSysSimpleColorModifier_Update.init(&hk_BSPSysSimpleColorModifier_Update);
	orig_BSMasterParticleSystem_ClearUnreferencedEmitters_C5E167.init(&hk_BSMasterParticleSystem_ClearUnreferencedEmitters_C5E167);
}*/