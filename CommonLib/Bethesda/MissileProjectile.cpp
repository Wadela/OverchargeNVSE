#include "MissileProjectile.hpp"
#include "BGSProjectile.hpp"
#include "bhkCharacterController.hpp"

void MissileProjectile::ProcessBounceNoCollision()
{
    NiAVObject* obj = this->Get3D();
    if (!obj)
        return;

    Projectile::ImpactData* impact = this->kImpacts.m_item;
    if (!impact)
        return;

    bhkCharacterController* chrCtrl = this->GetCharController();
    if (!chrCtrl)
        return;

    hkVector4 rawNormal(impact->kNormal.x, impact->kNormal.y, impact->kNormal.z, 0.0f);
    hkVector4 upBias(0.0f, 1.0f, 0.0f, 0.0f);

    float biasFactor = 0.2f;
    float nx = rawNormal.x() * (1.0f - biasFactor) + upBias.x() * biasFactor;
    float ny = rawNormal.y() * (1.0f - biasFactor) + upBias.y() * biasFactor;
    float nz = rawNormal.z() * (1.0f - biasFactor) + upBias.z() * biasFactor;

    float len = sqrt(nx * nx + ny * ny + nz * nz);
    if (len > 0.0001f) {
        nx /= len; ny /= len; nz /= len;
    }
    else {
        nx = 0.0f; ny = 1.0f; nz = 0.0f;
    }

    hkVector4 normal(nx, ny, nz, 0.0f);

    hkVector4& v = chrCtrl->kOutVelocity;

    float dot = v.x() * normal.x() + v.y() * normal.y() + v.z() * normal.z();

    float rx = v.x() - dot * normal.x();
    float ry = v.y() - dot * normal.y();
    float rz = v.z() - dot * normal.z();

    v = hkVector4(rx, ry, rz, v.w());

    chrCtrl->kDirection = v;

    auto reflectOrientation = [&](hkVector4& vec) {
        float d = vec.x() * normal.x() + vec.y() * normal.y() + vec.z() * normal.z();
        vec = hkVector4(vec.x() - 2.0f * d * normal.x(),
            vec.y() - 2.0f * d * normal.y(),
            vec.z() - 2.0f * d * normal.z(),
            vec.w());
        };

    reflectOrientation(chrCtrl->kUpVec);
    reflectOrientation(chrCtrl->kForwardVec);
    reflectOrientation(chrCtrl->kPushDelta);

    this->eFlags |= 0x4;
}


