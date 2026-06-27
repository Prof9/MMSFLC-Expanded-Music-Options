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

using namespace REFrameworkHelper;

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
                 {via::Language::English, u"Wave World BGM"},
             }},
            {L"ad4fcc9e-eccf-4e64-8afb-2286132ca680"_guid,
             {
                 {via::Language::English, u"Virus Battle BGM"},
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

    static std::shared_ptr<CustomPlaylistMenuItem> customPlaylistWaveWorld = std::make_unique<CustomPlaylistMenuItem>(
        L"9d3c9c45-c53f-42dd-b2fc-1beebdf63a28"_guid,
        L"45cf3ad4-b931-4885-a4fe-8e26be1b1475"_guid,
        &NEW_BGM_SELECTION_OPTIONS,
        &_bgmFieldSetting);
    static std::shared_ptr<CustomPlaylistMenuItem> customPlaylistVirusBattle = std::make_unique<CustomPlaylistMenuItem>(
        L"ad4fcc9e-eccf-4e64-8afb-2286132ca680"_guid,
        L"295acd67-b442-4328-af81-505199b3194a"_guid,
        &NEW_BGM_SELECTION_OPTIONS,
        &_bgmBattleSetting);

    // Extend audio menu
    std::vector<std::shared_ptr<MenuItem>> newMenuItems = {
        customPlaylistWaveWorld,
        customPlaylistVirusBattle,
    };
    new AudioMenuExtension(newMenuItems);

    // Override corePlayBgm
    hook(
        "app.cSound_Base.corePlayBgm",
        [](int argc, void **argv, auto...)
        {
            static bool isRecursiveCall = false;
            if (isRecursiveCall)
            {
                return REFRAMEWORK_HOOK_CALL_ORIGINAL;
            }
            // try
            //{

            Object sound = Object(argv[1]);
            std::uint32_t trigger = (std::uint32_t)(intptr_t)argv[2];
            Object origId2TriggerId = Object(argv[3]);
            std::uint32_t playerId = (std::uint32_t)(intptr_t)argv[4];

            std::uint8_t bgmPlayTypeFavorite = sound.get<std::uint8_t>("BgmPlayType_Favorite");

            if (trigger == 0xE98A8F76 || // SF1 - Ride On (SF1 Version)
                trigger == 0xBA31B7A6 || // SF2 - Ride On (SF2 Version)
                trigger == 0xDAE5756C)   // SF3 - Wave Battle (SF3 Version)
            {
                // Leverage existing function to select random song from playlist
                std::uint8_t bgmPlayType = sound.get<std::uint8_t>("_CurrentBgmPlayType");
                sound.get<std::uint8_t>("_CurrentBgmPlayType");
                sound.set<std::uint8_t>("_CurrentBgmPlayType", bgmPlayTypeFavorite);

                Object saveData = getSingleton("app.Launcher")["_saveData"];
                Object favoriteMusicList = saveData["favoriteMusicList"];
                saveData["favoriteMusicList"] = customPlaylistVirusBattle->m_favoritesList;

                isRecursiveCall = true;
                sound.call(
                    "corePlayBgm",
                    {
                        (void *)(intptr_t)trigger,
                        origId2TriggerId,
                        (void *)(intptr_t)playerId,
                    });
                isRecursiveCall = false;

                sound.set<std::uint8_t>("_CurrentBgmPlayType", bgmPlayType);
                saveData["favoriteMusicList"] = favoriteMusicList;

                return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
            }
            if (trigger == 0x59AF1699 || // SF1 - Wave World (SF1 Version)
                trigger == 0x296E097F || // SF2 - Wave World (SF2 Version)
                trigger == 0xBDAE2406    // SF3 - Wave World (SF3 Version)
            )
            {
            }

            Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
            std::uint8_t currentBgmPlayType = sound.get<std::uint8_t>("_CurrentBgmPlayType");

            Object soundMilkyManager = sound["_Manager"];
            Object extraResourceList = soundMilkyManager["_ExtraResourceList"];
            Object extraResource01 = extraResourceList.get(app::sound::SoundMilkyManager::ExtraResourceTypeEnum::ExtraResource01);
            Object extraResource02 = extraResourceList.get(app::sound::SoundMilkyManager::ExtraResourceTypeEnum::ExtraResource02);

            switch ((app::Launcher::BGMType)currentBgmPlayType)
            {
            case app::Launcher::BGMType::Exe1:
            case app::Launcher::BGMType::Exe2:
            case app::Launcher::BGMType::Exe3:
            case app::Launcher::BGMType::Exe4:
            case app::Launcher::BGMType::Exe4_5:
            case app::Launcher::BGMType::Exe5:
            case app::Launcher::BGMType::Exe5_2:
            case app::Launcher::BGMType::Exe6:
                break;
            case app::Launcher::BGMType::Favorite:
                break;
            }

            //}
            // catch (...)
            //{
            //    return REFRAMEWORK_HOOK_CALL_ORIGINAL;
            //}

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        nullptr);

    return true;
}
