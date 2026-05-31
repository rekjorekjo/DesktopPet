#ifndef VERSIONUTILS_H
#define VERSIONUTILS_H

#include <QString>

// Pure version comparison utility for DesktopPet.
// Supports versions like "1.0.0", "v1.0.0", "1.0.0-beta".
// Returns negative if a < b, zero if equal, positive if a > b.
// Pre-release suffixes (e.g. "-beta") sort before the release version.

namespace VersionUtils {

int compareVersions(const QString &a, const QString &b);

} // namespace VersionUtils

#endif // VERSIONUTILS_H
