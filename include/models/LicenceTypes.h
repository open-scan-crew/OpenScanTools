#define LICENCE_TYPES_H //Reverse line to activate file, currently desactivate because we switch open source
#ifndef LICENCE_TYPES_H

#include <unordered_set>
#include <unordered_map>

enum class LicenceVersion {Free, Educational, Standard, Portable};

enum class LicenceType {
	Empty, Free, TrialOnline, Online, OnlineFloating, Portable,
#ifdef FLOATING_LICENCE
	,Floating
#endif
};

static const std::unordered_map<LicenceVersion, std::unordered_set<LicenceVersion>> AssociatedLicenceVersionMap =
{
	{LicenceVersion::Free, {LicenceVersion::Portable}},
	{LicenceVersion::Standard,  {LicenceVersion::Free, LicenceVersion::Portable}},
	{LicenceVersion::Educational,  {LicenceVersion::Standard, LicenceVersion::Free, LicenceVersion::Portable}}
};

static bool s_isAccessibleVersion(LicenceVersion currentVersion, LicenceVersion compareVersion)
{
	if (currentVersion == compareVersion)
		return true;

	if (AssociatedLicenceVersionMap.find(currentVersion) != AssociatedLicenceVersionMap.end() &&
		AssociatedLicenceVersionMap.at(currentVersion).find(compareVersion) != AssociatedLicenceVersionMap.at(currentVersion).end())
		return true;
	return false;
}

#endif