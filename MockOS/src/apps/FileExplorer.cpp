#include "apps/FileExplorer.h"

#include <string>
#include <vector>

#include "ui/Fonts.h"
#include "ui/Theme.h"

namespace {

constexpr float SIDEBAR_WIDTH = 170.0f;
constexpr float HEADING_FONT_SIZE = 18.0f;

struct MockFile {
    const char* name;
    const char* type;
    const char* size;
};

struct Folder {
    const char* name;
    icons::LineIconFn icon;
    std::vector<MockFile> files;
};

const Folder FOLDERS[] = {
    {"Downloads", icons::download,
     {{"pesos_setup.exe", "Application", "48.2 MB"},
      {"CSOPESY_specs.pdf", "PDF Document", "1.2 MB"},
      {"wallpaper_pastel.png", "PNG Image", "3.4 MB"}}},
    {"Documents", icons::document,
     {{"Group_Report.docx", "Word Document", "256 KB"},
      {"Meeting_Notes.txt", "Text Document", "4 KB"},
      {"Budget.xlsx", "Excel Spreadsheet", "18 KB"}}},
    {"Music", icons::music,
     {{"lofi_study_beats.mp3", "MP3 Audio", "7.8 MB"}, {"pesos_startup.wav", "WAV Audio", "1.1 MB"}}},
    {"Pictures", icons::picture,
     {{"group_photo.jpg", "JPEG Image", "2.6 MB"}, {"screenshot_desktop.png", "PNG Image", "880 KB"}}},
    {"Videos", icons::video, {{"demo_recording.mp4", "MP4 Video", "96.4 MB"}}},
    {"This PC", icons::computer, {{"Local Disk (C:)", "Drive", "476 GB"}, {"Data (D:)", "Drive", "931 GB"}}},
    {"Network", icons::network, {{"PESOS-DESKTOP", "Computer", ""}, {"Shared Printer", "Printer", ""}}},
};
constexpr int FOLDER_COUNT = static_cast<int>(sizeof(FOLDERS) / sizeof(FOLDERS[0]));
constexpr int FIRST_LOCATION = 5;  // "This PC" and "Network" sit below a divider

}  // namespace

FileExplorer::FileExplorer()
    : App("File Explorer", "explorer.exe", icons::fileExplorer, ImVec2(180.0f, 80.0f), ImVec2(720.0f, 440.0f),
          ImVec2(540.0f, 380.0f)) {}

void FileExplorer::renderContents() {
    const float height = ImGui::GetContentRegionAvail().y;
    renderSidebar(height);
    ImGui::SameLine();
    theme::beginGlassCard("##folderCard", ImVec2(ImGui::GetContentRegionAvail().x, height));
    renderFolder();
    ImGui::EndChild();
}

void FileExplorer::renderSidebar(float height) {
    theme::beginGlassCard("##sidebar", ImVec2(SIDEBAR_WIDTH, height));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 2.0f));
    for (int i = 0; i < FOLDER_COUNT; ++i) {
        if (i == FIRST_LOCATION) {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        if (theme::sidebarItem(FOLDERS[i].name, FOLDERS[i].icon, i == folder_) && i != folder_) {
            folder_ = i;
            selectedFile_ = -1;
        }
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

// The folder's name and path, then its files.
void FileExplorer::renderFolder() {
    const Folder& folder = FOLDERS[folder_];

    ImGui::PushFont(fonts::semibold(), HEADING_FONT_SIZE);
    ImGui::TextUnformatted(folder.name);
    ImGui::PopFont();
    const std::string path = folder_ < FIRST_LOCATION ? std::string("This PC > ") + folder.name : folder.name;
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(theme::TEXT_MUTED));
    ImGui::TextUnformatted(path.c_str());
    ImGui::PopStyleColor();
    ImGui::Spacing();

    const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV |
                                  ImGuiTableFlags_SizingStretchProp;
    if (ImGui::BeginTable("files", 3, flags)) {
        ImGui::TableSetupColumn("Name", 0, 3.0f);
        ImGui::TableSetupColumn("Type", 0, 2.0f);
        ImGui::TableSetupColumn("Size", 0, 1.2f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(folder.files.size()); ++i) {
            const MockFile& file = folder.files[i];
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(i);
            if (ImGui::Selectable(file.name, i == selectedFile_, ImGuiSelectableFlags_SpanAllColumns)) {
                selectedFile_ = i;
            }
            ImGui::PopID();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(file.type);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(file.size);
        }
        ImGui::EndTable();
    }
}
