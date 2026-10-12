#include "ui/Fonts.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "imgui.h"
#include "resource.h"

namespace fonts {

namespace {

constexpr float DEFAULT_SIZE = 16.0f;

ImFont* regularFont = nullptr;
ImFont* semiboldFont = nullptr;
ImFont* monoFont = nullptr;

// The Poppins files are compiled into the .exe as resources (see MockOS.rc),
// so the program works no matter which folder it's started from.
ImFont* loadFromResource(int resourceId) {
    HRSRC resource = FindResourceW(nullptr, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
    if (resource == nullptr) {
        return nullptr;
    }
    void* data = LockResource(LoadResource(nullptr, resource));
    const int size = static_cast<int>(SizeofResource(nullptr, resource));
    if (data == nullptr || size == 0) {
        return nullptr;
    }
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;  // the data belongs to the .exe; ImGui must not free it
    return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(data, size, DEFAULT_SIZE, &config);
}

}  // namespace

void load() {
    // The first font added becomes ImGui's default.
    regularFont = loadFromResource(IDR_FONT_POPPINS_REGULAR);
    semiboldFont = loadFromResource(IDR_FONT_POPPINS_SEMIBOLD);
    monoFont = ImGui::GetIO().Fonts->AddFontDefaultVector();

    // If the resources are missing, fall back to the built-in font rather than crash.
    if (regularFont == nullptr) {
        regularFont = monoFont;
        ImGui::GetIO().FontDefault = monoFont;
    }
    if (semiboldFont == nullptr) {
        semiboldFont = regularFont;
    }
}

ImFont* regular() {
    return regularFont;
}

ImFont* semibold() {
    return semiboldFont;
}

ImFont* mono() {
    return monoFont;
}

}  // namespace fonts
