#include <Enums_Internal.hpp>

#include <cassert>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdfloat>
#include <thread>
#include <utility>
#include <vector>

#include <Windows.h>

#include <reframework/API.hpp>

#include "AudioMenuExtension.hpp"
#include "Guid.hpp"
#include "MenuItem.hpp"
#include "MessageManager.hpp"
#include "MessageUtility.hpp"

#define CONFIG_DIR "reframework\\data\\ExpandedMusicOptions"
#define CONFIG_FILENAME CONFIG_DIR "\\config.bin"

static std::int32_t _bgmFieldSetting = -1;
static std::int32_t _bgmBattleSetting = -1;
static std::int32_t _bgmSetting3 = -1;
static std::int32_t _bgmSetting4 = -1;
static std::int32_t _bgmSetting5 = -1;
static std::int32_t _bgmSetting6 = -1;

static bool _doResizeSoundList = false;

static int _newSoundOptionsIdx = -1;

static const size_t FLUENT_SCROLL_LIST_OFFSET_BAR_PATH = 0x2D0;

static const std::vector<Guid> NEW_BGM_SELECTION_OPTIONS = {
    L"8095b996-deab-419a-b034-5d21fa0aee21"_guid, // 100%
    L"3a33dbca-f51f-4c3e-bf72-9041cdec608b"_guid, // 200%
    L"2aebe1ff-927d-4131-8c1a-9e4030feb348"_guid, // 300%
    L"6049ca56-9c49-4300-9b99-78f16e32504b"_guid, // 400%
    L"81468531-34ae-4e65-812d-ccb48b78b158"_guid, // 500%
};

static std::vector<MenuItem> const NEW_MENU_ITEMS = {
    MenuItem{
        .m_nameGuid = L"9dd4d43c-4b86-48f5-9b6f-d1816656760f"_guid,
        .m_descriptionGuid = L"45cf3ad4-b931-4885-a4fe-8e26be1b1475"_guid,
        .m_valueNames = &NEW_BGM_SELECTION_OPTIONS,
        .m_value = &_bgmFieldSetting,
    },
    MenuItem{
        .m_nameGuid = L"aeed2fdf-2495-4fe7-9b9d-afdd92a1a631"_guid,
        .m_descriptionGuid = L"295acd67-b442-4328-af81-505199b3194a"_guid,
        .m_valueNames = &NEW_BGM_SELECTION_OPTIONS,
        .m_value = &_bgmBattleSetting,
    },
    MenuItem{
        .m_nameGuid = L"5327f1b2-51f8-4b7d-9510-d90b2cedd1f2"_guid,
        .m_descriptionGuid = L"7acea6b1-a466-45b9-890a-74ecbe0adee1"_guid,
        .m_valueNames = &NEW_BGM_SELECTION_OPTIONS,
        .m_value = &_bgmSetting3,
    },
    MenuItem{
        .m_nameGuid = L"6e45bcd6-0f76-4576-8b42-9ac87762328e"_guid,
        .m_descriptionGuid = L"0a12f712-9d90-42b3-bb90-f123dedfc6b3"_guid,
        .m_valueNames = &NEW_BGM_SELECTION_OPTIONS,
        .m_value = &_bgmSetting4,
    },
    MenuItem{
        .m_nameGuid = L"43293d60-97ef-4e61-9de7-abafe61c42d8"_guid,
        .m_descriptionGuid = L"f0a6ffdd-413a-4a22-8382-8fc8a6e4e159"_guid,
        .m_valueNames = &NEW_BGM_SELECTION_OPTIONS,
        .m_value = &_bgmSetting5,
    },
    MenuItem{
        .m_nameGuid = L"f3240d92-582d-4013-ab24-ae564c872d85"_guid,
        .m_descriptionGuid = L"1c09fcc8-3f61-4ed1-bd4c-f2742f84c432"_guid,
        .m_valueNames = &NEW_BGM_SELECTION_OPTIONS,
        .m_value = &_bgmSetting6,
    },
};

static reframework::API::ManagedObject *_battlePlayer = nullptr;
static reframework::API::ManagedObject *_menuTblOption = nullptr;

extern "C" __declspec(dllexport) void reframework_plugin_required_version(REFrameworkPluginVersion *version)
{
    version->major = REFRAMEWORK_PLUGIN_VERSION_MAJOR;
    version->minor = REFRAMEWORK_PLUGIN_VERSION_MINOR;
    version->patch = REFRAMEWORK_PLUGIN_VERSION_PATCH;
}

extern "C" __declspec(dllexport) bool reframework_plugin_initialize(const REFrameworkPluginInitializeParam *param)
{
    auto &api = reframework::API::initialize(param);
    auto tdb = api->tdb();

    // Load settings from file
    std::ifstream configFile;
    configFile.open(CONFIG_FILENAME, std::fstream::in | std::fstream::binary);
    if (configFile.is_open())
    {
        try
        {
            configFile.read((char *)&_bgmFieldSetting, sizeof(_bgmFieldSetting));
            configFile.read((char *)&_bgmBattleSetting, sizeof(_bgmFieldSetting));
        }
        catch (...)
        {
            api->log_error("Failed to load settings from file!");
        }
        configFile.close();
    }

    // Wait for Launcher to initialize
    reframework::API::ManagedObject *launcher = nullptr;
    while (true)
    {
        launcher = api->get_managed_singleton("app.Launcher");
        if (launcher != nullptr)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    assert(launcher != nullptr);

    // Get MessageManager
    void *messageManagerNativeObject = api->get_native_singleton("via.gui.MessageManager");
    assert(messageManagerNativeObject != nullptr);
    MessageManager messageManager(messageManagerNativeObject);

    new AudioMenuExtension(NEW_MENU_ITEMS);

    return true;
}
