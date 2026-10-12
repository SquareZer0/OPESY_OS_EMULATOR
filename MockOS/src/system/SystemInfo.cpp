#include "system/SystemInfo.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>
#include <intrin.h>
#include <GL/gl.h>

#include <cstring>
#include <thread>

namespace {

constexpr int MAX_DRIVES = 16;  // PhysicalDrive0 to PhysicalDrive15
const wchar_t* const CPU_KEY = L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
const wchar_t* const BIOS_KEY = L"HARDWARE\\DESCRIPTION\\System\\BIOS";

std::string trim(const std::string& text) {
    const size_t first = text.find_first_not_of(" \t");
    if (first == std::string::npos) {
        return "";
    }
    const size_t last = text.find_last_not_of(" \t");
    return text.substr(first, last - first + 1);
}

std::string toUtf8(const wchar_t* text) {
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1) {
        return "";
    }
    std::vector<char> buffer(size);
    WideCharToMultiByte(CP_UTF8, 0, text, -1, buffer.data(), size, nullptr, nullptr);
    return buffer.data();
}

std::string readRegistryString(const wchar_t* key, const wchar_t* name) {
    wchar_t buffer[256] = {};
    DWORD size = sizeof(buffer);
    if (RegGetValueW(HKEY_LOCAL_MACHINE, key, name, RRF_RT_REG_SZ, nullptr, buffer, &size) != ERROR_SUCCESS) {
        return "";
    }
    return trim(toUtf8(buffer));
}

DWORD readRegistryNumber(const wchar_t* key, const wchar_t* name) {
    DWORD value = 0;
    DWORD size = sizeof(value);
    RegGetValueW(HKEY_LOCAL_MACHINE, key, name, RRF_RT_REG_DWORD, nullptr, &value, &size);
    return value;
}

// The CPU's own name for itself, e.g. "Intel(R) Core(TM) i7-14700F".
std::string cpuBrand() {
    int registers[4] = {};
    __cpuid(registers, 0x80000000);
    if (static_cast<unsigned int>(registers[0]) < 0x80000004) {
        return readRegistryString(CPU_KEY, L"ProcessorNameString");
    }
    char brand[49] = {};
    for (int i = 0; i < 3; ++i) {
        __cpuid(registers, 0x80000002 + i);
        std::memcpy(brand + i * 16, registers, sizeof(registers));
    }
    return trim(brand);
}

int countCores() {
    DWORD length = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length);
    if (length == 0) {
        return 0;
    }
    std::vector<char> buffer(length);
    if (!GetLogicalProcessorInformationEx(RelationProcessorCore,
                                          reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer.data()),
                                          &length)) {
        return 0;
    }
    // One variable-size entry per physical core.
    int cores = 0;
    for (DWORD offset = 0; offset < length; ++cores) {
        offset += reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer.data() + offset)->Size;
    }
    return cores;
}

unsigned long long installedMemoryKB() {
    ULONGLONG kilobytes = 0;
    if (GetPhysicallyInstalledSystemMemory(&kilobytes)) {
        return kilobytes;
    }
    MEMORYSTATUSEX status = {};
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys / 1024;
}

const char* busName(STORAGE_BUS_TYPE bus) {
    switch (bus) {
    case BusTypeNvme: return "NVMe";
    case BusTypeSata: return "SATA";
    case BusTypeAta: return "IDE";
    case BusTypeUsb: return "USB";
    case BusTypeScsi:
    case BusTypeSas: return "SCSI";
    case BusTypeRAID: return "RAID";
    case BusTypeSd:
    case BusTypeMmc: return "SD";
    default: return "Disk";
    }
}

std::vector<DriveInfo> queryDrives() {
    std::vector<DriveInfo> drives;
    for (int i = 0; i < MAX_DRIVES; ++i) {
        wchar_t path[32];
        swprintf_s(path, L"\\\\.\\PhysicalDrive%d", i);
        // Opening with no access rights is enough to ask for the model and
        // size, and works without admin rights.
        HANDLE handle = CreateFileW(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (handle == INVALID_HANDLE_VALUE) {
            continue;
        }

        DriveInfo drive;
        STORAGE_PROPERTY_QUERY query = {};
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;
        BYTE descriptor[1024] = {};
        DWORD bytes = 0;
        if (DeviceIoControl(handle, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), descriptor,
                            sizeof(descriptor), &bytes, nullptr)) {
            const auto* device = reinterpret_cast<const STORAGE_DEVICE_DESCRIPTOR*>(descriptor);
            auto field = [&](DWORD offset) {
                return (offset != 0 && offset < bytes) ? trim(reinterpret_cast<const char*>(descriptor + offset))
                                                       : std::string();
            };
            std::string vendor = field(device->VendorIdOffset);
            const std::string product = field(device->ProductIdOffset);
            if (vendor == "NVMe" || product.rfind(vendor, 0) == 0) {
                vendor.clear();  // not a real vendor name, or already part of the product name
            }
            drive.model = vendor.empty() ? product : vendor + " " + product;
            drive.bus = busName(device->BusType);
        }

        BYTE geometry[256] = {};
        if (DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, nullptr, 0, geometry, sizeof(geometry), &bytes,
                            nullptr)) {
            drive.sizeGB = reinterpret_cast<const DISK_GEOMETRY_EX*>(geometry)->DiskSize.QuadPart / 1e9;
        }
        CloseHandle(handle);

        if (!drive.model.empty()) {
            drives.push_back(drive);
        }
    }
    return drives;
}

std::string gpuName() {
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    if (renderer == nullptr) {
        return "";
    }
    // NVIDIA appends "/PCIe/SSE2"; keep just the card's name.
    std::string name = renderer;
    const size_t slash = name.find('/');
    if (slash != std::string::npos) {
        name.erase(slash);
    }
    return trim(name);
}

}  // namespace

SystemInfo querySystemInfo() {
    SystemInfo info;
    info.cpuName = cpuBrand();
    info.cpuMHz = static_cast<int>(readRegistryNumber(CPU_KEY, L"~MHz"));
    info.cores = countCores();
    info.threads = static_cast<int>(std::thread::hardware_concurrency());
    info.memoryKB = installedMemoryKB();
    info.boardMaker = readRegistryString(BIOS_KEY, L"BaseBoardManufacturer");
    info.boardModel = readRegistryString(BIOS_KEY, L"BaseBoardProduct");
    info.gpuName = gpuName();
    info.drives = queryDrives();
    return info;
}
