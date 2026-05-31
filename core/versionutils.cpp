#include "versionutils.h"

#include <QStringList>

namespace VersionUtils {

// Parse a version string into (major, minor, patch, isPreRelease).
// Strips leading "v" and any suffix after a hyphen (treated as pre-release).
static void parseVersion(const QString &version, int &major, int &minor, int &patch, bool &preRelease)
{
    QString v = version.trimmed();
    if (v.startsWith('v', Qt::CaseInsensitive)) {
        v = v.mid(1);
    }

    preRelease = false;
    int hyphenIdx = v.indexOf('-');
    if (hyphenIdx >= 0) {
        preRelease = true;
        v = v.left(hyphenIdx);
    }

    QStringList parts = v.split('.');
    major = parts.size() > 0 ? parts[0].toInt() : 0;
    minor = parts.size() > 1 ? parts[1].toInt() : 0;
    patch = parts.size() > 2 ? parts[2].toInt() : 0;
}

int compareVersions(const QString &a, const QString &b)
{
    int aMajor, aMinor, aPatch;
    bool aPre;
    int bMajor, bMinor, bPatch;
    bool bPre;

    parseVersion(a, aMajor, aMinor, aPatch, aPre);
    parseVersion(b, bMajor, bMinor, bPatch, bPre);

    if (aMajor != bMajor) return (aMajor < bMajor) ? -1 : 1;
    if (aMinor != bMinor) return (aMinor < bMinor) ? -1 : 1;
    if (aPatch != bPatch) return (aPatch < bPatch) ? -1 : 1;

    // Same numeric version: release > pre-release
    if (aPre != bPre) return aPre ? -1 : 1;

    return 0;
}

} // namespace VersionUtils
