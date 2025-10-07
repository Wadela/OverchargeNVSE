#pragma once
#include "TESPackage.hpp"
#include "TESPackageData.hpp"

class TESFollowPackageData : public TESPackageData {
public:
	TESFollowPackageData();
	~TESFollowPackageData();
	TESPackage::PackageLocation* endLocation;
	Float32	flt008;
};