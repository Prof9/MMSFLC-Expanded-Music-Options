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

#define CONFIG_FILE(name) ("reframework\\data\\ExpandedMusicOptions\\" name)

static struct
{
    std::int32_t m_settingRealWorldBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingWaveRoadBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingWaveSpaceBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingDungeonBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingNormalBattleBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingBossBattleBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingLastBattleBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingWinnerBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingLoserBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingIncidentBgmRealWorld = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingIncidentBgmWaveWorld = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingMiscellaneousBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingMainMenuBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
    std::int32_t m_settingDlcBgmLoneliness = false;
    std::int32_t m_settingDlcBgmAstroWave = false;
    std::int32_t m_settingDlcBgmAnthemOfTheSolitary = false;
    std::int32_t m_settingDlcBgmCheerfulIndoors = false;
} s_Settings;

static const std::vector<MenuItem::Option> OPTIONS_COMMON = {
    {.m_nameGuid = L"c564c411-216f-498f-9c53-459408c675d5"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange),
     .m_descriptionGuid = L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid},
    {.m_nameGuid = L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::MusicOff),
     .m_descriptionGuid = L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid},
    {.m_nameGuid = L"b46d7983-3acf-40a4-8c71-3f2bfb7d7b89"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysOriginal),
     .m_descriptionGuid = L"bf4b7003-90ae-45fa-bcda-66d72fa818e5"_guid},
    {.m_nameGuid = L"773ac3ba-06fc-489c-85f1-589efb6b546f"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysArranged),
     .m_descriptionGuid = L"7fd7bc1a-88ee-4631-bbec-a731d86deed5"_guid},
    {.m_nameGuid = L"c12ba05a-7bbc-4e46-b510-53a361c262f4"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::PreferMix),
     .m_descriptionGuid = L"501e2a85-77ba-4f87-a7a1-b672e5674e54"_guid},
    {.m_nameGuid = L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Playlist),
     .m_descriptionGuid = L"297584f6-5da9-4131-99fd-365f2d8746ac"_guid},
    {.m_nameGuid = L"c5451ab5-0f50-45c0-8368-de71c13aa44b"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Favorites),
     .m_descriptionGuid = L"99b3bbca-167c-416c-aa9d-1349dac7b784"_guid},
};
static const std::vector<MenuItem::Option> OPTIONS_NORMAL_BATTLE = {
    {.m_nameGuid = L"c564c411-216f-498f-9c53-459408c675d5"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange),
     .m_descriptionGuid = L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid},
    {.m_nameGuid = L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::MusicOff),
     .m_descriptionGuid = L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid},
    {.m_nameGuid = L"b46d7983-3acf-40a4-8c71-3f2bfb7d7b89"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysOriginal),
     .m_descriptionGuid = L"bf4b7003-90ae-45fa-bcda-66d72fa818e5"_guid},
    {.m_nameGuid = L"773ac3ba-06fc-489c-85f1-589efb6b546f"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysArranged),
     .m_descriptionGuid = L"7fd7bc1a-88ee-4631-bbec-a731d86deed5"_guid},
    {.m_nameGuid = L"c12ba05a-7bbc-4e46-b510-53a361c262f4"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::PreferMix),
     .m_descriptionGuid = L"501e2a85-77ba-4f87-a7a1-b672e5674e54"_guid},
    {.m_nameGuid = L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Playlist),
     .m_descriptionGuid = L"297584f6-5da9-4131-99fd-365f2d8746ac"_guid},
    {.m_nameGuid = L"c5451ab5-0f50-45c0-8368-de71c13aa44b"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Favorites),
     .m_descriptionGuid = L"99b3bbca-167c-416c-aa9d-1349dac7b784"_guid},
    {.m_nameGuid = L"0c174efc-4ff0-42ea-98d2-c0956ca6c5ba"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Field),
     .m_descriptionGuid = L"53ff37ca-a276-470c-b310-8c0ad3ce1349"_guid},
    {.m_nameGuid = L"c7e77ba9-a882-4d37-a11b-e5900d140d51"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::FieldOriginal),
     .m_descriptionGuid = L"18b51d97-8636-4bb6-b6b7-4e5a2c46afd5"_guid},
    {.m_nameGuid = L"91937c25-8452-45d8-a735-aa569695badb"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::FieldArranged),
     .m_descriptionGuid = L"bdbd21a1-9136-4639-bf6a-bda1fdc18e64"_guid},
};
static const std::vector<MenuItem::Option> OPTIONS_WINNER_LOSER = {
    {.m_nameGuid = L"c564c411-216f-498f-9c53-459408c675d5"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange),
     .m_descriptionGuid = L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid},
    {.m_nameGuid = L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::MusicOff),
     .m_descriptionGuid = L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid},
    {.m_nameGuid = L"b46d7983-3acf-40a4-8c71-3f2bfb7d7b89"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysOriginal),
     .m_descriptionGuid = L"bf4b7003-90ae-45fa-bcda-66d72fa818e5"_guid},
    {.m_nameGuid = L"773ac3ba-06fc-489c-85f1-589efb6b546f"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysArranged),
     .m_descriptionGuid = L"7fd7bc1a-88ee-4631-bbec-a731d86deed5"_guid},
    {.m_nameGuid = L"c12ba05a-7bbc-4e46-b510-53a361c262f4"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::PreferMix),
     .m_descriptionGuid = L"501e2a85-77ba-4f87-a7a1-b672e5674e54"_guid},
    {.m_nameGuid = L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Playlist),
     .m_descriptionGuid = L"297584f6-5da9-4131-99fd-365f2d8746ac"_guid},
    {.m_nameGuid = L"c5451ab5-0f50-45c0-8368-de71c13aa44b"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Favorites),
     .m_descriptionGuid = L"99b3bbca-167c-416c-aa9d-1349dac7b784"_guid},
    {.m_nameGuid = L"0a1089a5-f79f-45d4-9329-f159d2d4fdc2"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Battle),
     .m_descriptionGuid = L"22089bb3-1ed9-498f-bb7b-b10063cdc380"_guid},
};
static const std::vector<MenuItem::Option> OPTIONS_MAIN_MENU = {
    {.m_nameGuid = L"c564c411-216f-498f-9c53-459408c675d5"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange),
     .m_descriptionGuid = L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid},
    {.m_nameGuid = L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::MusicOff),
     .m_descriptionGuid = L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid},
    {.m_nameGuid = L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Playlist),
     .m_descriptionGuid = L"297584f6-5da9-4131-99fd-365f2d8746ac"_guid},
    {.m_nameGuid = L"c5451ab5-0f50-45c0-8368-de71c13aa44b"_guid,
     .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Favorites),
     .m_descriptionGuid = L"99b3bbca-167c-416c-aa9d-1349dac7b784"_guid},
};
static const std::vector<MenuItem::Option> OPTIONS_DLC_BGM = {
    {.m_nameGuid = L"0f2bff24-8e0d-4e6d-851f-0ec36763e53e"_guid,
     .m_value = false},
    {.m_nameGuid = L"2e96a0fe-baa7-45b6-8bff-f60852d94c3e"_guid,
     .m_value = true},
};

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
    configFile.open(CONFIG_FILE("config.bin"), std::fstream::in | std::fstream::binary);
    if (configFile.is_open())
    {
        try
        {
            configFile.read((char *)&s_Settings, sizeof(s_Settings));
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
            {L"8debd0e1-ad1f-4afc-80a7-d1013ad86e89"_guid,
             {
                 {via::Language::English, u"Real World BGM"},
             }},
            {L"c80c7d4e-fef6-4a17-839d-c983ceaafee8"_guid,
             {
                 {via::Language::English, u"Select what tracks will play in the Real World."},
             }},
            {L"9d3c9c45-c53f-42dd-b2fc-1beebdf63a28"_guid,
             {
                 {via::Language::English, u"Wave Road BGM"},
             }},
            {L"8c8efb06-f15d-4844-baea-1ea64ebba80a"_guid,
             {
                 {via::Language::English, u"Select what tracks will play on Wave Roads."},
             }},
            {L"ac468c10-eb46-4101-8dcf-922c9407a09a"_guid,
             {
                 {via::Language::English, u"Wave Space BGM"},
             }},
            {L"bb07dbc0-d6df-4719-b32c-ecf9f2eb7ccd"_guid,
             {
                 {via::Language::English, u"Select what tracks will play in Comp Spaces, EM Wave Spaces,\nCyber Cores, and non-numbered Noise Waves."},
             }},
            {L"00214dd1-02cf-46be-9d60-7137604c6daa"_guid,
             {
                 {via::Language::English, u"Dungeon BGM"},
             }},
            {L"b91d4a7b-c8d9-46b8-ae1e-e9cb1d632015"_guid,
             {
                 {via::Language::English, u"Select what tracks will play in dungeons, the Sky Wave,\nthe Astro Wave, and numbered Noise Waves."},
             }},
            {L"ad4fcc9e-eccf-4e64-8afb-2286132ca680"_guid,
             {
                 {via::Language::English, u"Normal Battle BGM"},
             }},
            {L"13dec11f-c01f-492c-bd69-c7b0da9fb32f"_guid,
             {
                 {via::Language::English, u"Select what tracks will play in regular battles and online Matches."},
             }},
            {L"a95b7459-663c-4fa6-af8d-a3294250efbc"_guid,
             {
                 {via::Language::English, u"Boss Battle BGM"},
             }},
            {L"f9548ad4-63c6-44e3-9291-a2d0000ad2b7"_guid,
             {
                 {via::Language::English, u"Select what tracks will play in boss battles."},
             }},
            {L"b9144bd0-98da-439a-9da4-021c5365d153"_guid,
             {
                 {via::Language::English, u"Last Battle BGM"},
             }},
            {L"a14c26cf-289d-4c1a-a8bc-33735eb110a6"_guid,
             {
                 {via::Language::English, u"Select what tracks will play in the final battle."},
             }},
            {L"6fb1bc61-3b8b-4826-a409-edc9e374f200"_guid,
             {
                 {via::Language::English, u"Winner! BGM"},
             }},
            {L"c89aa7f5-56ec-4dc1-b7e0-29a60836e0e7"_guid,
             {
                 {via::Language::English, u"Select what tracks will play at the end of most won battles.\n* Some options will not apply to battles without rewards."},
             }},
            {L"3744b966-9ad0-4267-bb8e-a961a0f146eb"_guid,
             {
                 {via::Language::English, u"Loser... BGM"},
             }},
            {L"bd4eee29-1654-46ea-b94d-1ebcec877a4f"_guid,
             {
                 {via::Language::English, u"Select what tracks will play at the end of lost battles in online Matches."},
             }},
            {L"5e8401f5-8a28-4efb-9311-2e8fc505d79e"_guid,
             {
                 {via::Language::English, u"Incident BGM (Real World)"},
             }},
            {L"115ddb26-3a7a-4e90-8b60-ce581697d8ef"_guid,
             {
                 {via::Language::English, u"Select what tracks will play during incidents in the Real World."},
             }},
            {L"6dd57050-d840-4f5c-bda4-8868441dc1aa"_guid,
             {
                 {via::Language::English, u"Incident BGM (Wave World)"},
             }},
            {L"9074ac1f-c5c5-4776-a97b-83066bf0b07c"_guid,
             {
                 {via::Language::English, u"Select what tracks will play during incidents in the Wave World."},
             }},
            {L"65c3c7c7-fa93-45fa-9cbf-f3a4cf23b862"_guid,
             {
                 {via::Language::English, u"Miscellaneous BGM"},
             }},
            {L"10428196-f3b5-452f-94f8-50e8a38139ce"_guid,
             {
                 {via::Language::English, u"Select what tracks will play during miscellaneous events."},
             }},
            {L"3cc5ab6c-4a2a-4cb0-8816-845aefbf01ab"_guid,
             {
                 {via::Language::English, u"Main Menu BGM"},
             }},
            {L"bc71a2d1-54df-41b0-a2ee-670a4eb6e0a9"_guid,
             {
                 {via::Language::English, u"Select what tracks will play on the collection's Main Menu."},
             }},
            {L"73c512c7-6d5d-405e-96d8-d9bcc7eff8d7"_guid,
             {
                 {via::Language::English, u"DLC BGM - Loneliness"},
             }},
            {L"238a92fc-c37b-4319-b2e1-f40cb6915c33"_guid,
             {
                 {via::Language::English, u"Select whether to play Loneliness - Summer Shade Version in-game."},
             }},
            {L"28a49ef5-fc0c-48c5-a8eb-60c92ba20cc8"_guid,
             {
                 {via::Language::English, u"DLC BGM - Astro Wave"},
             }},
            {L"5491219b-1b6e-4e37-a894-e065e37d7bb2"_guid,
             {
                 {via::Language::English, u"Select whether to play Astro Wave - Brilliant Expanse Version in-game."},
             }},
            {L"cc097b76-9d12-40db-91ee-dff3e0852892"_guid,
             {
                 {via::Language::English, u"DLC BGM - Anthem of the Solitary"},
             }},
            {L"8313007b-6258-4f73-82e3-07952e8d9831"_guid,
             {
                 {via::Language::English, u"Select whether to play Anthem of the Solitary - Howling Void Version in-game."},
             }},
            {L"31d45ae4-128e-4172-820f-4fd2b06510a5"_guid,
             {
                 {via::Language::English, u"DLC BGM - Cheerful Indoors"},
             }},
            {L"61511466-dd94-4c00-ae88-4a8878002c8b"_guid,
             {
                 {via::Language::English, u"Select whether to play Cheerful Indoors - School Days Version in-game."},
             }},
            {L"c564c411-216f-498f-9c53-459408c675d5"_guid,
             {
                 {via::Language::English, u"No Change"},
             }},
            {L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid,
             {
                 {via::Language::English, u"Play the normal track as determined by BGM Selection."},
             }},
            {L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
             {
                 {via::Language::English, u"Music Off"},
             }},
            {L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid,
             {
                 {via::Language::English, u"Background music is disabled for this type of track."},
             }},
            {L"b46d7983-3acf-40a4-8c71-3f2bfb7d7b89"_guid,
             {
                 {via::Language::English, u"Always Original"},
             }},
            {L"bf4b7003-90ae-45fa-bcda-66d72fa818e5"_guid,
             {
                 {via::Language::English, u"Always play the original version of the track."},
             }},
            {L"773ac3ba-06fc-489c-85f1-589efb6b546f"_guid,
             {
                 {via::Language::English, u"Always Arranged"},
             }},
            {L"7fd7bc1a-88ee-4631-bbec-a731d86deed5"_guid,
             {
                 {via::Language::English, u"Always play the arranged version of the track."},
             }},
            {L"c12ba05a-7bbc-4e46-b510-53a361c262f4"_guid,
             {
                 {via::Language::English, u"Prefer Mix <ICON LAUNCHER_DECIDE>"},
             }},
            {L"501e2a85-77ba-4f87-a7a1-b672e5674e54"_guid,
             {
                 {via::Language::English, u"Always play the original or arranged track if that version is marked as Favorite.\n* This list is shared for all track types."},
             }},
            {L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
             {
                 {via::Language::English, u"Playlist <ICON LAUNCHER_DECIDE>"},
             }},
            {L"297584f6-5da9-4131-99fd-365f2d8746ac"_guid,
             {
                 {via::Language::English, u"Always play tracks randomly selected from a\ndedicated list of Favorites for this type of track."},
             }},
            {L"c5451ab5-0f50-45c0-8368-de71c13aa44b"_guid,
             {
                 {via::Language::English, u"Favorites <ICON LAUNCHER_DECIDE>"},
             }},
            {L"99b3bbca-167c-416c-aa9d-1349dac7b784"_guid,
             {
                 {via::Language::English, u"Always play tracks randomly selected from your regular list of Favorites."},
             }},
            {L"0c174efc-4ff0-42ea-98d2-c0956ca6c5ba"_guid,
             {
                 {via::Language::English, u"Field"},
             }},
            {L"53ff37ca-a276-470c-b310-8c0ad3ce1349"_guid,
             {
                 {via::Language::English, u"Play the background music from the Field Screen uninterrupted.\n* Does not apply to online Matches."},
             }},
            {L"c7e77ba9-a882-4d37-a11b-e5900d140d51"_guid,
             {
                 {via::Language::English, u"Field Original"},
             }},
            {L"18b51d97-8636-4bb6-b6b7-4e5a2c46afd5"_guid,
             {
                 {via::Language::English, u"Play the original version of the background music from the Field Screen uninterrupted.\n* Does not apply to online Matches."},
             }},
            {L"91937c25-8452-45d8-a735-aa569695badb"_guid,
             {
                 {via::Language::English, u"Field Arranged"},
             }},
            {L"bdbd21a1-9136-4639-bf6a-bda1fdc18e64"_guid,
             {
                 {via::Language::English, u"Play the arranged version of the background music from the Field Screen uninterrupted.\n* Does not apply to online Matches."},
             }},
            {L"0a1089a5-f79f-45d4-9329-f159d2d4fdc2"_guid,
             {
                 {via::Language::English, u"Battle"},
             }},
            {L"22089bb3-1ed9-498f-bb7b-b10063cdc380"_guid,
             {
                 {via::Language::English, u"Play the background music from the Battle Screen uninterrupted."},
             }},
        },
        via::Language::English);

    CustomPlaylistMenuItem::setPreferMixPlaylistFileName(CONFIG_FILE("playlist_prefer_mix.bin"));
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemRealWorldBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_real_world.bin"),
        L"8debd0e1-ad1f-4afc-80a7-d1013ad86e89"_guid,
        L"c80c7d4e-fef6-4a17-839d-c983ceaafee8"_guid,
        &OPTIONS_COMMON,
        &s_Settings.m_settingRealWorldBgm);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemWaveRoadBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_wave_road.bin"),
        L"9d3c9c45-c53f-42dd-b2fc-1beebdf63a28"_guid,
        L"8c8efb06-f15d-4844-baea-1ea64ebba80a"_guid,
        &OPTIONS_COMMON,
        &s_Settings.m_settingWaveRoadBgm);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemWaveSpaceBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_wave_space.bin"),
        L"ac468c10-eb46-4101-8dcf-922c9407a09a"_guid,
        L"bb07dbc0-d6df-4719-b32c-ecf9f2eb7ccd"_guid,
        &OPTIONS_COMMON,
        &s_Settings.m_settingWaveSpaceBgm);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemDungeonBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_dungeon.bin"),
        L"00214dd1-02cf-46be-9d60-7137604c6daa"_guid,
        L"b91d4a7b-c8d9-46b8-ae1e-e9cb1d632015"_guid,
        &OPTIONS_COMMON,
        &s_Settings.m_settingDungeonBgm);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemNormalBattleBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_normal_battle.bin"),
        L"ad4fcc9e-eccf-4e64-8afb-2286132ca680"_guid,
        L"13dec11f-c01f-492c-bd69-c7b0da9fb32f"_guid,
        &OPTIONS_NORMAL_BATTLE,
        &s_Settings.m_settingNormalBattleBgm);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemBossBattleBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_boss_battle.bin"),
        L"a95b7459-663c-4fa6-af8d-a3294250efbc"_guid,
        L"f9548ad4-63c6-44e3-9291-a2d0000ad2b7"_guid,
        &OPTIONS_COMMON,
        &s_Settings.m_settingBossBattleBgm);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemLastBattleBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_last_battle.bin"),
        L"b9144bd0-98da-439a-9da4-021c5365d153"_guid,
        L"a14c26cf-289d-4c1a-a8bc-33735eb110a6"_guid,
        &OPTIONS_COMMON,
        &s_Settings.m_settingLastBattleBgm);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemWinnerBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_winner.bin"),
        L"6fb1bc61-3b8b-4826-a409-edc9e374f200"_guid,
        L"c89aa7f5-56ec-4dc1-b7e0-29a60836e0e7"_guid,
        &OPTIONS_WINNER_LOSER,
        &s_Settings.m_settingWinnerBgm);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemLoserBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_loser.bin"),
        L"3744b966-9ad0-4267-bb8e-a961a0f146eb"_guid,
        L"bd4eee29-1654-46ea-b94d-1ebcec877a4f"_guid,
        &OPTIONS_WINNER_LOSER,
        &s_Settings.m_settingLoserBgm);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemIncidentBgmRealWorld = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_incident_real_world.bin"),
        L"5e8401f5-8a28-4efb-9311-2e8fc505d79e"_guid,
        L"115ddb26-3a7a-4e90-8b60-ce581697d8ef"_guid,
        &OPTIONS_COMMON,
        &s_Settings.m_settingIncidentBgmRealWorld);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemIncidentBgmWaveWorld = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_incident_wave_world.bin"),
        L"6dd57050-d840-4f5c-bda4-8868441dc1aa"_guid,
        L"9074ac1f-c5c5-4776-a97b-83066bf0b07c"_guid,
        &OPTIONS_COMMON,
        &s_Settings.m_settingIncidentBgmWaveWorld);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemMiscellaneousBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_miscellaneous.bin"),
        L"65c3c7c7-fa93-45fa-9cbf-f3a4cf23b862"_guid,
        L"10428196-f3b5-452f-94f8-50e8a38139ce"_guid,
        &OPTIONS_COMMON,
        &s_Settings.m_settingMiscellaneousBgm);
    static std::shared_ptr<CustomPlaylistMenuItem> s_menuItemMainMenuBgm = std::make_unique<CustomPlaylistMenuItem>(
        CONFIG_FILE("playlist_main_menu.bin"),
        L"3cc5ab6c-4a2a-4cb0-8816-845aefbf01ab"_guid,
        L"bc71a2d1-54df-41b0-a2ee-670a4eb6e0a9"_guid,
        &OPTIONS_MAIN_MENU,
        &s_Settings.m_settingMainMenuBgm);
    static std::shared_ptr<MenuItem> s_menuItemDlcBgmLoneliness = std::make_unique<MenuItem>(
        L"73c512c7-6d5d-405e-96d8-d9bcc7eff8d7"_guid,
        L"238a92fc-c37b-4319-b2e1-f40cb6915c33"_guid,
        &OPTIONS_DLC_BGM,
        &s_Settings.m_settingDlcBgmLoneliness);
    static std::shared_ptr<MenuItem> s_menuItemDlcBgmAstroWave = std::make_unique<MenuItem>(
        L"28a49ef5-fc0c-48c5-a8eb-60c92ba20cc8"_guid,
        L"5491219b-1b6e-4e37-a894-e065e37d7bb2"_guid,
        &OPTIONS_DLC_BGM,
        &s_Settings.m_settingDlcBgmAstroWave);
    static std::shared_ptr<MenuItem> s_menuItemDlcBgmAnthemOfTheSolitary = std::make_unique<MenuItem>(
        L"cc097b76-9d12-40db-91ee-dff3e0852892"_guid,
        L"8313007b-6258-4f73-82e3-07952e8d9831"_guid,
        &OPTIONS_DLC_BGM,
        &s_Settings.m_settingDlcBgmAnthemOfTheSolitary);
    static std::shared_ptr<MenuItem> s_menuItemDlcBgmCheerfulIndoors = std::make_unique<MenuItem>(
        L"31d45ae4-128e-4172-820f-4fd2b06510a5"_guid,
        L"61511466-dd94-4c00-ae88-4a8878002c8b"_guid,
        &OPTIONS_DLC_BGM,
        &s_Settings.m_settingDlcBgmCheerfulIndoors);

    // Extend audio menu
    std::vector<std::shared_ptr<MenuItem>> newMenuItems = {
        s_menuItemRealWorldBgm,
        s_menuItemWaveRoadBgm,
        s_menuItemWaveSpaceBgm,
        s_menuItemDungeonBgm,
        s_menuItemNormalBattleBgm,
        s_menuItemBossBattleBgm,
        s_menuItemLastBattleBgm,
        s_menuItemWinnerBgm,
        s_menuItemLoserBgm,
        s_menuItemIncidentBgmRealWorld,
        s_menuItemIncidentBgmWaveWorld,
        s_menuItemMiscellaneousBgm,
        s_menuItemMainMenuBgm,
        s_menuItemDlcBgmLoneliness,
        s_menuItemDlcBgmAstroWave,
        s_menuItemDlcBgmAnthemOfTheSolitary,
        s_menuItemDlcBgmCheerfulIndoors,
    };
    new AudioMenuExtension(newMenuItems);

    static std::vector<bool> s_isPlayerArranged;
    static Object s_currentSound;

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
                saveData["favoriteMusicList"] = s_menuItemNormalBattleBgm->getCustomPlaylist();

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
                // Leverage existing function to select random song from playlist
                std::uint8_t bgmPlayType = sound.get<std::uint8_t>("_CurrentBgmPlayType");
                sound.get<std::uint8_t>("_CurrentBgmPlayType");
                sound.set<std::uint8_t>("_CurrentBgmPlayType", bgmPlayTypeFavorite);

                Object saveData = getSingleton("app.Launcher")["_saveData"];
                Object favoriteMusicList = saveData["favoriteMusicList"];
                saveData["favoriteMusicList"] = s_menuItemWaveRoadBgm->getCustomPlaylist();

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

    hook(
        "app.cSound_Base.baseInit",
        [](int argc, void **argv, auto...)
        {
            s_currentSound = Object(argv[1]);
            s_isPlayerArranged.resize(0);
            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        nullptr);

    hook(
        "app.cSound_Base.baseTerminate",
        nullptr,
        [](auto...)
        {
            s_currentSound = nullptr;
            s_isPlayerArranged.resize(0);
        });

    static bool s_disableEnableBgmArrangeHook = false;
    hook(
        "app.sound.SoundMilkyManager.set_EnableBgmArrange(System.Boolean)",
        [](int argc, void **argv, auto...)
        {
            if (s_currentSound != nullptr && !s_disableEnableBgmArrangeHook)
            {
                bool isArranged = (bool)(intptr_t)argv[2];

                std::uint32_t playerId = s_currentSound.get<std::uint32_t>("_CurrentPlayBgmPlayerId");
                if (playerId >= s_isPlayerArranged.size())
                {
                    s_isPlayerArranged.resize(playerId + 1);
                }
                s_isPlayerArranged[playerId] = isArranged;
            }

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        nullptr);

    hook(
        "app.cSound_Base.updateBgmPlayType",
        [](int argc, void **argv, auto...)
        {
            Object sound = Object(argv[1]);
            std::uint32_t playerId = sound.get<std::uint32_t>("_CurrentPlayBgmPlayerId");
            bool isArranged = playerId < s_isPlayerArranged.size() ? s_isPlayerArranged[playerId] : false;

            s_disableEnableBgmArrangeHook = true;
            sound["_Manager"].call("set_EnableBgmArrange", {(void *)(intptr_t)isArranged});
            s_disableEnableBgmArrangeHook = false;

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        nullptr);

    return true;
}
