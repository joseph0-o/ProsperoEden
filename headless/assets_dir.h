// SPDX-License-Identifier: GPL-3.0-or-later
// The game files folder: keys/, firmware/ and roms/ in the folder discovered automatically across
// USB/external drives (/mnt/usb0/homebrew/prosperoeden first) or chosen in Settings > Game files.
// Where the rest lives: storage_paths.h; what is saved: settings_store.h.
#pragma once
#include <string>
#include <string_view>

#ifndef SETTINGS_STORE_H_MOCK
#include "settings_store.h"
#endif
#include "storage_paths.h"

namespace Eden {

inline bool HasAssetsSubfolders(const std::string& dir) {
    return DirectoryExists(dir + "/keys") || DirectoryExists(dir + "/firmware") || DirectoryExists(dir + "/roms");
}

// Resolves the best default/candidate folder across all connected USB/ext drives and internal storage.
inline std::string ResolveDefaultAssetsDir() {
    if (!FilesystemAccess()) return kDefaultAssetsDir;
    const std::string keys = "/keys/prod.keys";
    for (const char* candidate : kCandidateAssetsDirs) {
        if (FileExists(std::string{candidate} + keys)) return std::string{candidate};
    }
    const std::string legacy = AppFile("assets");
    if (FileExists(legacy + keys)) return legacy;
    if (FileExists(AppDir() + keys)) return AppDir();

    for (const char* candidate : kCandidateAssetsDirs) {
        const std::string path{candidate};
        if (path == kDataDir) continue;
        if (HasAssetsSubfolders(path)) return path;
    }
    for (const char* candidate : kCandidateAssetsDirs) {
        const std::string path{candidate};
        if (path == kDataDir) continue;
        if (DirectoryExists(path)) return path;
    }
    return kDefaultAssetsDir;
}

// Resolves the active game files folder:
// 1. A user-saved custom folder if it contains keys/prod.keys.
// 2. The first candidate in kCandidateAssetsDirs (/mnt/usb0/homebrew/prosperoeden, usb1..7, ext0..1,
//    etc.) that contains keys/prod.keys (enabling Plug & Play when moving the drive between consoles).
// 3. A user-saved custom folder or external candidate folder that exists on disk.
// 4. Fallback to kDefaultAssetsDir (/mnt/usb0/homebrew/prosperoeden).
inline std::string ResolveAssetsDir() {
    if (!FilesystemAccess()) return kDefaultAssetsDir;
    const std::string keys = "/keys/prod.keys";
    const std::string saved = LoadSavedAssetsDir();

    // 1. Honor an explicit custom saved path if it holds prod.keys.
    if (!saved.empty() && saved != kDataDir && FileExists(saved + keys)) return saved;

    // 2. Search all candidate paths in priority order for prod.keys.
    for (const char* candidate : kCandidateAssetsDirs) {
        if (FileExists(std::string{candidate} + keys)) return std::string{candidate};
    }
    const std::string legacy = AppFile("assets");
    if (FileExists(legacy + keys)) return legacy;
    if (FileExists(AppDir() + keys)) return AppDir();

    // 3. If no path has prod.keys yet, prefer an existing custom saved path, then any existing
    //    external candidate folder with keys/firmware/roms subfolders, then any existing external
    //    candidate folder.
    if (!saved.empty() && saved != kDataDir && DirectoryExists(saved)) return saved;
    for (const char* candidate : kCandidateAssetsDirs) {
        const std::string path{candidate};
        if (path == kDataDir) continue;
        if (HasAssetsSubfolders(path)) return path;
    }
    for (const char* candidate : kCandidateAssetsDirs) {
        const std::string path{candidate};
        if (path == kDataDir) continue;
        if (DirectoryExists(path)) return path;
    }
    if (!saved.empty() && DirectoryExists(saved)) return saved;
    return kDefaultAssetsDir;
}

inline std::string& MutableAssetsDir() {
    static std::string directory = ResolveAssetsDir();
    return directory;
}

// The game files folder in use by this process.
inline const std::string& AssetsDir() {
    return MutableAssetsDir();
}

// Updates the active game files folder immediately in the running process.
inline void SetAssetsDir(std::string_view directory) {
    if (ValidAssetsDir(directory)) {
        MutableAssetsDir() = std::string(directory);
    }
}

// Re-runs automatic discovery and updates the active game files folder.
inline const std::string& RefreshAssetsDir() {
    MutableAssetsDir() = ResolveAssetsDir();
    return MutableAssetsDir();
}

inline std::string AssetsPath(std::string_view child) { return AssetsDir() + "/" + std::string(child); }
} // namespace Eden
