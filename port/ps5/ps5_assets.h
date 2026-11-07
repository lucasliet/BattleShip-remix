#pragma once

#include <string>

namespace ssb64 {

/**
 * Makes sure the game archive in the title folder matches this build, extracting it with Torch from
 * the user's ROM in the same folder when it is missing or stale.
 * @param titleDir the title folder (/app0): ROM, archive, Torch's config under assets/
 * @return true when a current archive is in place
 */
bool Ps5EnsureGameArchive(const std::string& titleDir);

/** Temporary HTTPS check (ps5_http_probe.cpp). */
void Ps5RunHttpProbe(const std::string& titleDir);

} // namespace ssb64
