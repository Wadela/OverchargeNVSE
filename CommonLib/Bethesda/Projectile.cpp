#include "BGSProjectile.hpp"
#include "Projectile.hpp"
#include "Actor.hpp"

Projectile* Projectile::CloneProjectile()
{
	BGSProjectile* baseProjectile = reinterpret_cast<BGSProjectile*>(this->pkObjectReference);

	Actor* act = reinterpret_cast<Actor*>(this->pSourceRef);
	CombatController* combatCtrl = act->GetCombatController();
	bool alwaysHit = 1 ? this->eFlags | 0x1000 : 0;
	bool ignoreGrav = 1 ? this->eFlags | 0x40 : 0;
	Projectile* newProj = CdeclCall<Projectile*>(0x9BCA60, 
		baseProjectile, 
		act,
		combatCtrl, 
		this->pSourceWeapon, 
		this->kPosition, this->kRotation.z, this->kRotation.x, 
		0, 
		this->pLiveGrenadeTargetRef, 
		alwaysHit, ignoreGrav, 
		this->fSpreadZ, this->fSpreadX, 
		this->pkParentCell);

	return newProj;
}