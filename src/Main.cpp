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
#include "CustomPlaylistMenuItem.hpp"
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
    L"c564c411-216f-498f-9c53-459408c675d5"_guid, // Default
    L"225f5de3-b958-4c53-b33c-3d2ff2bad06f"_guid, // Original
    L"37ea887b-9d75-42b2-bd0b-2806cb6d688a"_guid, // Arranged
    L"09d503da-bb7a-49c7-b920-844b91b4cba1"_guid, // Favorites
    L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid, // Playlist [A]
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

    messageManager.createAndLoadMessages(
        {
            {L"9d3c9c45-c53f-42dd-b2fc-1beebdf63a28"_guid,
             {
                 {via::Language::English, u"Field BGM"},
             }},
            {L"ad4fcc9e-eccf-4e64-8afb-2286132ca680"_guid,
             {
                 {via::Language::English, u"Battle BGM"},
             }},
            {L"c564c411-216f-498f-9c53-459408c675d5"_guid,
             {
                 {via::Language::English, u"Default"},
             }},
            {L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
             {
                 {via::Language::English, u"Playlist <ICON LAUNCHER_DECIDE>"},
             }},
        },
        via::Language::English);

    // Extend audio menu
    static std::vector<std::unique_ptr<MenuItem>> NEW_MENU_ITEMS = []
    {
        std::vector<std::unique_ptr<MenuItem>> items;
        items.emplace_back(std::make_unique<CustomPlaylistMenuItem>(
            L"9d3c9c45-c53f-42dd-b2fc-1beebdf63a28"_guid,
            L"45cf3ad4-b931-4885-a4fe-8e26be1b1475"_guid,
            &NEW_BGM_SELECTION_OPTIONS,
            &_bgmFieldSetting));
        items.emplace_back(std::make_unique<CustomPlaylistMenuItem>(
            L"ad4fcc9e-eccf-4e64-8afb-2286132ca680"_guid,
            L"295acd67-b442-4328-af81-505199b3194a"_guid,
            &NEW_BGM_SELECTION_OPTIONS,
            &_bgmBattleSetting));
        items.emplace_back(std::make_unique<MenuItem>(
            L"5327f1b2-51f8-4b7d-9510-d90b2cedd1f2"_guid,
            L"7acea6b1-a466-45b9-890a-74ecbe0adee1"_guid,
            &NEW_BGM_SELECTION_OPTIONS,
            &_bgmSetting3));
        items.emplace_back(std::make_unique<MenuItem>(
            L"6e45bcd6-0f76-4576-8b42-9ac87762328e"_guid,
            L"0a12f712-9d90-42b3-bb90-f123dedfc6b3"_guid,
            &NEW_BGM_SELECTION_OPTIONS,
            &_bgmSetting4));
        items.emplace_back(std::make_unique<MenuItem>(
            L"43293d60-97ef-4e61-9de7-abafe61c42d8"_guid,
            L"f0a6ffdd-413a-4a22-8382-8fc8a6e4e159"_guid,
            &NEW_BGM_SELECTION_OPTIONS,
            &_bgmSetting5));
        items.emplace_back(std::make_unique<MenuItem>(
            L"f3240d92-582d-4013-ab24-ae564c872d85"_guid,
            L"1c09fcc8-3f61-4ed1-bd4c-f2742f84c432"_guid,
            &NEW_BGM_SELECTION_OPTIONS,
            &_bgmSetting6));
        return items;
    }();
    new AudioMenuExtension(NEW_MENU_ITEMS);

    return true;
}
