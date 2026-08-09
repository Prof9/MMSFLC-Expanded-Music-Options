#include <Enums_Internal.hpp>

#include <memory>
#include <vector>

#include "Common.hpp"
#include "CustomPlaylistMenuItem.hpp"
#include "Guid.hpp"
#include "MenuItem.hpp"
#include "MusicSystemMod.hpp"
#include "REFrameworkHelper.hpp"
#include "ReplaceMusicMenuItem.hpp"

using namespace REFrameworkHelper;

const std::vector<MenuItem::Option> MusicSystemMod::OPTIONS_COMMON = {
	{.m_nameGuid = L"c564c411-216f-498f-9c53-459408c675d5"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange),
	 .m_descriptionGuid = L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid},
	{.m_nameGuid = L"b46d7983-3acf-40a4-8c71-3f2bfb7d7b89"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysOriginal),
	 .m_descriptionGuid = L"bf4b7003-90ae-45fa-bcda-66d72fa818e5"_guid},
	{.m_nameGuid = L"773ac3ba-06fc-489c-85f1-589efb6b546f"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysArranged),
	 .m_descriptionGuid = L"7fd7bc1a-88ee-4631-bbec-a731d86deed5"_guid},
	{.m_nameGuid = L"c12ba05a-7bbc-4e46-b510-53a361c262f4"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::PreferredMix),
	 .m_descriptionGuid = L"501e2a85-77ba-4f87-a7a1-b672e5674e54"_guid},
	{.m_nameGuid = L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Playlist),
	 .m_descriptionGuid = L"297584f6-5da9-4131-99fd-365f2d8746ac"_guid},
	{.m_nameGuid = L"c5451ab5-0f50-45c0-8368-de71c13aa44b"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Favorites),
	 .m_descriptionGuid = L"99b3bbca-167c-416c-aa9d-1349dac7b784"_guid},
	{.m_nameGuid = L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::MusicOff),
	 .m_descriptionGuid = L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid},
};
const std::vector<MenuItem::Option> MusicSystemMod::OPTIONS_NORMAL_BATTLE = {
	{.m_nameGuid = L"c564c411-216f-498f-9c53-459408c675d5"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange),
	 .m_descriptionGuid = L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid},
	{.m_nameGuid = L"b46d7983-3acf-40a4-8c71-3f2bfb7d7b89"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysOriginal),
	 .m_descriptionGuid = L"bf4b7003-90ae-45fa-bcda-66d72fa818e5"_guid},
	{.m_nameGuid = L"773ac3ba-06fc-489c-85f1-589efb6b546f"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysArranged),
	 .m_descriptionGuid = L"7fd7bc1a-88ee-4631-bbec-a731d86deed5"_guid},
	{.m_nameGuid = L"c12ba05a-7bbc-4e46-b510-53a361c262f4"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::PreferredMix),
	 .m_descriptionGuid = L"501e2a85-77ba-4f87-a7a1-b672e5674e54"_guid},
	{.m_nameGuid = L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Playlist),
	 .m_descriptionGuid = L"297584f6-5da9-4131-99fd-365f2d8746ac"_guid},
	{.m_nameGuid = L"c5451ab5-0f50-45c0-8368-de71c13aa44b"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Favorites),
	 .m_descriptionGuid = L"99b3bbca-167c-416c-aa9d-1349dac7b784"_guid},
	// {.m_nameGuid = L"0c174efc-4ff0-42ea-98d2-c0956ca6c5ba"_guid,
	//  .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Field),
	//  .m_descriptionGuid = L"53ff37ca-a276-470c-b310-8c0ad3ce1349"_guid},
	// {.m_nameGuid = L"c7e77ba9-a882-4d37-a11b-e5900d140d51"_guid,
	//  .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::FieldOriginal),
	//  .m_descriptionGuid = L"18b51d97-8636-4bb6-b6b7-4e5a2c46afd5"_guid},
	// {.m_nameGuid = L"91937c25-8452-45d8-a735-aa569695badb"_guid,
	//  .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::FieldArranged),
	//  .m_descriptionGuid = L"bdbd21a1-9136-4639-bf6a-bda1fdc18e64"_guid},
	{.m_nameGuid = L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::MusicOff),
	 .m_descriptionGuid = L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid},
};
const std::vector<MenuItem::Option> MusicSystemMod::OPTIONS_WINNER_LOSER = {
	{.m_nameGuid = L"c564c411-216f-498f-9c53-459408c675d5"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange),
	 .m_descriptionGuid = L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid},
	{.m_nameGuid = L"b46d7983-3acf-40a4-8c71-3f2bfb7d7b89"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysOriginal),
	 .m_descriptionGuid = L"bf4b7003-90ae-45fa-bcda-66d72fa818e5"_guid},
	{.m_nameGuid = L"773ac3ba-06fc-489c-85f1-589efb6b546f"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysArranged),
	 .m_descriptionGuid = L"7fd7bc1a-88ee-4631-bbec-a731d86deed5"_guid},
	{.m_nameGuid = L"c12ba05a-7bbc-4e46-b510-53a361c262f4"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::PreferredMix),
	 .m_descriptionGuid = L"501e2a85-77ba-4f87-a7a1-b672e5674e54"_guid},
	{.m_nameGuid = L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Playlist),
	 .m_descriptionGuid = L"297584f6-5da9-4131-99fd-365f2d8746ac"_guid},
	{.m_nameGuid = L"c5451ab5-0f50-45c0-8368-de71c13aa44b"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Favorites),
	 .m_descriptionGuid = L"99b3bbca-167c-416c-aa9d-1349dac7b784"_guid},
	// {.m_nameGuid = L"0a1089a5-f79f-45d4-9329-f159d2d4fdc2"_guid,
	//  .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Battle),
	//  .m_descriptionGuid = L"22089bb3-1ed9-498f-bb7b-b10063cdc380"_guid},
	{.m_nameGuid = L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::MusicOff),
	 .m_descriptionGuid = L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid},
};
const std::vector<MenuItem::Option> MusicSystemMod::OPTIONS_MAIN_MENU = {
	{.m_nameGuid = L"c564c411-216f-498f-9c53-459408c675d5"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange),
	 .m_descriptionGuid = L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid},
	{.m_nameGuid = L"36fac9cf-514b-45f7-b015-b5b54b4817c3"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Playlist),
	 .m_descriptionGuid = L"297584f6-5da9-4131-99fd-365f2d8746ac"_guid},
	{.m_nameGuid = L"c5451ab5-0f50-45c0-8368-de71c13aa44b"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::Favorites),
	 .m_descriptionGuid = L"99b3bbca-167c-416c-aa9d-1349dac7b784"_guid},
	{.m_nameGuid = L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::MusicOff),
	 .m_descriptionGuid = L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid},
};
const std::vector<MenuItem::Option> MusicSystemMod::OPTIONS_MAIN_MENU_RESTRICTED = {
	{.m_nameGuid = L"c564c411-216f-498f-9c53-459408c675d5"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange),
	 .m_descriptionGuid = L"efab3c28-9b48-442f-9fcb-7cf75b46b7bc"_guid},
	{.m_nameGuid = L"33fffdce-f115-4cd5-ade2-2a168f47d19a"_guid,
	 .m_value = std::to_underlying(CustomPlaylistMenuItem::Option::MusicOff),
	 .m_descriptionGuid = L"7f561d13-08c8-43c5-af02-4248afa489e4"_guid},
};
const std::vector<MenuItem::Option> MusicSystemMod::OPTIONS_DLC_BGM = {
	{.m_nameGuid = L"0f2bff24-8e0d-4e6d-851f-0ec36763e53e"_guid,
	 .m_value = false},
	{.m_nameGuid = L"2e96a0fe-baa7-45b6-8bff-f60852d94c3e"_guid,
	 .m_value = true},
};

void MusicSystemMod::buildMenu()
{
	CustomPlaylistMenuItem::setPreferredMixPlaylistFileName(CONFIG_FILE("playlist_preferred_mix.bin"));

	MusicSystemMod::Settings defaultSettings;

	s_menuItemRealWorldBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_real_world.bin"),
		L"8debd0e1-ad1f-4afc-80a7-d1013ad86e89"_guid,
		L"c80c7d4e-fef6-4a17-839d-c983ceaafee8"_guid,
		&OPTIONS_COMMON,
		&s_settings.RealWorldBgm,
		defaultSettings.RealWorldBgm);
	s_menuItemWaveRoadBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_wave_road.bin"),
		L"9d3c9c45-c53f-42dd-b2fc-1beebdf63a28"_guid,
		L"8c8efb06-f15d-4844-baea-1ea64ebba80a"_guid,
		&OPTIONS_COMMON,
		&s_settings.WaveRoadBgm,
		defaultSettings.WaveRoadBgm);
	s_menuItemWaveSpaceBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_wave_space.bin"),
		L"ac468c10-eb46-4101-8dcf-922c9407a09a"_guid,
		L"bb07dbc0-d6df-4719-b32c-ecf9f2eb7ccd"_guid,
		&OPTIONS_COMMON,
		&s_settings.WaveSpaceBgm,
		defaultSettings.WaveSpaceBgm);
	s_menuItemDungeonBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_dungeon.bin"),
		L"00214dd1-02cf-46be-9d60-7137604c6daa"_guid,
		L"b91d4a7b-c8d9-46b8-ae1e-e9cb1d632015"_guid,
		&OPTIONS_COMMON,
		&s_settings.DungeonBgm,
		defaultSettings.DungeonBgm);
	s_menuItemNormalBattleBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_normal_battle.bin"),
		L"ad4fcc9e-eccf-4e64-8afb-2286132ca680"_guid,
		L"13dec11f-c01f-492c-bd69-c7b0da9fb32f"_guid,
		&OPTIONS_NORMAL_BATTLE,
		&s_settings.NormalBattleBgm,
		defaultSettings.NormalBattleBgm);
	s_menuItemBossBattleBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_boss_battle.bin"),
		L"a95b7459-663c-4fa6-af8d-a3294250efbc"_guid,
		L"f9548ad4-63c6-44e3-9291-a2d0000ad2b7"_guid,
		&OPTIONS_COMMON,
		&s_settings.BossBattleBgm,
		defaultSettings.BossBattleBgm);
	s_menuItemLastBattleBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_last_battle.bin"),
		L"b9144bd0-98da-439a-9da4-021c5365d153"_guid,
		L"a14c26cf-289d-4c1a-a8bc-33735eb110a6"_guid,
		&OPTIONS_COMMON,
		&s_settings.LastBattleBgm,
		defaultSettings.LastBattleBgm);
	s_menuItemWinnerBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_winner.bin"),
		L"6fb1bc61-3b8b-4826-a409-edc9e374f200"_guid,
		L"c89aa7f5-56ec-4dc1-b7e0-29a60836e0e7"_guid,
		&OPTIONS_WINNER_LOSER,
		&s_settings.WinnerBgm,
		defaultSettings.WinnerBgm);
	s_menuItemLoserBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_loser.bin"),
		L"3744b966-9ad0-4267-bb8e-a961a0f146eb"_guid,
		L"bd4eee29-1654-46ea-b94d-1ebcec877a4f"_guid,
		&OPTIONS_WINNER_LOSER,
		&s_settings.LoserBgm,
		defaultSettings.LoserBgm);
	s_menuItemIncidentBgmRealWorld = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_incident_real_world.bin"),
		L"5e8401f5-8a28-4efb-9311-2e8fc505d79e"_guid,
		L"115ddb26-3a7a-4e90-8b60-ce581697d8ef"_guid,
		&OPTIONS_COMMON,
		&s_settings.IncidentBgmRealWorld,
		defaultSettings.IncidentBgmRealWorld);
	s_menuItemIncidentBgmWaveWorld = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_incident_wave_world.bin"),
		L"6dd57050-d840-4f5c-bda4-8868441dc1aa"_guid,
		L"9074ac1f-c5c5-4776-a97b-83066bf0b07c"_guid,
		&OPTIONS_COMMON,
		&s_settings.IncidentBgmWaveWorld,
		defaultSettings.IncidentBgmWaveWorld);
	s_menuItemMiscellaneousBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_miscellaneous.bin"),
		L"65c3c7c7-fa93-45fa-9cbf-f3a4cf23b862"_guid,
		L"10428196-f3b5-452f-94f8-50e8a38139ce"_guid,
		&OPTIONS_COMMON,
		&s_settings.MiscellaneousBgm,
		defaultSettings.MiscellaneousBgm);
	s_menuItemTitleScreenBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_title_screen.bin"),
		L"8a38a4b9-0527-4b36-abde-c96ca132db30"_guid,
		L"69a9799a-898f-40c8-b1e0-550eb37256e7"_guid,
		&OPTIONS_COMMON,
		&s_settings.TitleScreenBgm,
		defaultSettings.TitleScreenBgm);
	s_menuItemMainMenuBgm = std::make_unique<CustomPlaylistMenuItem>(
		CONFIG_FILE("playlist_main_menu.bin"),
		L"3cc5ab6c-4a2a-4cb0-8816-845aefbf01ab"_guid,
		L"bc71a2d1-54df-41b0-a2ee-670a4eb6e0a9"_guid,
		hasMainMenuBgm() ? &OPTIONS_MAIN_MENU : &OPTIONS_MAIN_MENU_RESTRICTED,
		&s_settings.MainMenuBgm,
		defaultSettings.MainMenuBgm);
	s_menuItemDlcBgmLoneliness = std::make_unique<ReplaceMusicMenuItem>(
		L"73c512c7-6d5d-405e-96d8-d9bcc7eff8d7"_guid,
		L"238a92fc-c37b-4319-b2e1-f40cb6915c33"_guid,
		&OPTIONS_DLC_BGM,
		&s_settings.DlcBgmLoneliness,
		defaultSettings.DlcBgmLoneliness,
		app::DLCContentsManager::DLC_TYPE::DLC01);
	s_menuItemDlcBgmAstroWave = std::make_unique<ReplaceMusicMenuItem>(
		L"28a49ef5-fc0c-48c5-a8eb-60c92ba20cc8"_guid,
		L"5491219b-1b6e-4e37-a894-e065e37d7bb2"_guid,
		&OPTIONS_DLC_BGM,
		&s_settings.DlcBgmAstroWave,
		defaultSettings.DlcBgmAstroWave,
		app::DLCContentsManager::DLC_TYPE::DLC01);
	s_menuItemDlcBgmAnthemOfTheSolitary = std::make_unique<ReplaceMusicMenuItem>(
		L"cc097b76-9d12-40db-91ee-dff3e0852892"_guid,
		L"8313007b-6258-4f73-82e3-07952e8d9831"_guid,
		&OPTIONS_DLC_BGM,
		&s_settings.DlcBgmAnthemOfTheSolitary,
		defaultSettings.DlcBgmAnthemOfTheSolitary,
		app::DLCContentsManager::DLC_TYPE::DLC01);
	s_menuItemDlcBgmCheerfulIndoors = std::make_unique<ReplaceMusicMenuItem>(
		L"31d45ae4-128e-4172-820f-4fd2b06510a5"_guid,
		L"61511466-dd94-4c00-ae88-4a8878002c8b"_guid,
		&OPTIONS_DLC_BGM,
		&s_settings.DlcBgmCheerfulIndoors,
		defaultSettings.DlcBgmCheerfulIndoors,
		app::DLCContentsManager::DLC_TYPE::DLC01);

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
		s_menuItemTitleScreenBgm,
		s_menuItemMainMenuBgm,
		s_menuItemDlcBgmLoneliness,
		s_menuItemDlcBgmAstroWave,
		s_menuItemDlcBgmAnthemOfTheSolitary,
		s_menuItemDlcBgmCheerfulIndoors,
	};
	s_audioMenuExtension = std::make_unique<AudioMenuExtension>(newMenuItems, saveSettings);

	s_musicSettingInfoList = {
		// DLC replace settings have higher priority
		{
			.Type = MusicSettingType::ReplaceMusic,
			.TriggerIDs = {
				0x522A7C35, // SF1: Loneliness
				0xC2ABB29D, // SF2: Loneliness
				0x29E0CC15, // SF3: Loneliness
			},
			.MenuItem = s_menuItemDlcBgmLoneliness,
			.NewTriggerID = 0x114CEC27, // Loneliness - Summer Shade Version
			.RequiredDLC = app::DLCContentsManager::DLC_TYPE::DLC01,
		},
		{
			.Type = MusicSettingType::ReplaceMusic,
			.TriggerIDs = {
				0x052ABAC7, // SF3: Astro Wave
			},
			.MenuItem = s_menuItemDlcBgmAstroWave,
			.NewTriggerID = 0x2DD44285, // Astro Wave - Brilliant Expanse Version
			.RequiredDLC = app::DLCContentsManager::DLC_TYPE::DLC01,
		},
		{
			.Type = MusicSettingType::ReplaceMusic,
			.TriggerIDs = {
				0x4C86D9BA, // SF2: Anthem of the Solitary
				0xE1AD6054, // SF3: Anthem of the Solitary
			},
			.MenuItem = s_menuItemDlcBgmAnthemOfTheSolitary,
			.NewTriggerID = 0x071CFE13, // Anthem of the Solitary - Howling Void Version
			.RequiredDLC = app::DLCContentsManager::DLC_TYPE::DLC01,
		},
		{
			.Type = MusicSettingType::ReplaceMusic,
			.TriggerIDs = {
				0xE994DBDE, // SF1: Cheerful Indoors
				0x412B3875, // SF2: Cheerful Indoors
				0x295EEC52, // SF3: Cheerful Indoors
			},
			.MenuItem = s_menuItemDlcBgmCheerfulIndoors,
			.NewTriggerID = 0x7D3E1480, // Cheerful Indoors - School Days Version
			.RequiredDLC = app::DLCContentsManager::DLC_TYPE::DLC01,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0xFCF8ECEB, // SF1: Hometown (SF1 Version)
				0xE994DBDE, // SF1: Cheerful Indoors
				0x735B58D3, // SF1: Science
				0x35662562, // SF1: Homeroom
				0xCCF80A84, // SF1: Time Square
				0x5054502B, // SF1: Dream Island
				0x55A434B4, // SF1: Space Station
				0x2DB7EBDD, // SF1: Zero Gravity
				0x851924DA, // SF2: Hometown (SF2 Version)
				0x412B3875, // SF2: Cheerful Indoors
				0x814B669D, // SF2: Grizzly Peak
				0xAEF78761, // SF2: Loch Mess
				0x4458D85B, // SF2: Wilshire Hills
				0x8B492BB6, // SF2: Whazzap
				0x81BF1D76, // SF3: Hometown (SF3 Version)
				0x295EEC52, // SF3: Cheerful Indoors
				0x836FCD1E, // SF3: Smile: Spica Mall
				0xC03EB07B, // SF3: WBG Studios
				0xDFD1059F, // SF3: Heloha Alohaha!
				0x3CC9753C, // SF3: Protectors of Peace
				0x097909CB, // SF3: Hideout
				0x3858C74A, // SF3: Echo Elementary
				0x3829A658, // SF3: Let's Go Shopping
			},
			.MenuItem = s_menuItemRealWorldBgm,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0x59AF1699, // SF1: Wave World (SF1 Version)
				0x296E097F, // SF2: Wave World (SF2 Version)
				0xBDAE2406, // SF3: Wave World (SF3 Version)
			},
			.MenuItem = s_menuItemWaveRoadBgm,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0x7C2959CC, // SF1: Cyber World (SF1 Version)
				0x70B3B9F9, // SF2: Wave Square
				0x590C820E, // SF3: Cyber World (SF3 Version)
				0xD2E1A491, // SF3: Noise World
			},
			.MenuItem = s_menuItemWaveSpaceBgm,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0x286104C8, // SF1: Rampaging Truck!
				0xD8A9FB8F, // SF1: Rocket Shooter
				0x62A67D21, // SF1: Study Time!
				0xF4569294, // SF1: Anaconda
				0x3A401CA7, // SF1: Dust Crash
				0x9B5A8F36, // SF1: Final Battlefield
				0xA0FB31B6, // SF1: Space!?
				0x33950EBE, // SF2: Ghost Mayhem
				0xF026001A, // SF2: Snowstorm
				0x48C05D86, // SF2: Lake Exploration
				0x23D9B5B8, // SF2: Ancient Omen
				0x6B2F9A95, // SF2: Bermuda Maze
				0x8E438530, // SF2: Mu
				0x82C99A2B, // SF2: Requiem of Ruination
				0xC33A51ED, // SF2: Sky Wave
				0x052ABAC7, // SF3: Astro Wave
				0x1173E658, // SF3: Noise Wave
				0x323FB3B9, // SF3: Bonds Beyond the Stars
				0xE6CBFBC4, // SF3: Rocket Emergency!
				0x2902BD52, // SF3: Live Stage
				0x2E098CC4, // SF3: Rocked and Rolled
				0x33FE775F, // SF3: Crimson Machine
				0x33EE8743, // SF3: Memory Splinter: Meteor Server
			},
			.MenuItem = s_menuItemDungeonBgm,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0xE98A8F76, // SF1: Ride On (SF1 Version)
				0xBA31B7A6, // SF2: Ride On (SF2 Version)
				0x9C314038, // SF2: Road to Victory (SF2 Version)
				0xDAE5756C, // SF3: Wave Battle (SF3 Version)
				0xF75D6831, // SF3: Road to Victory (SF3 Version)
			},
			.MenuItem = s_menuItemNormalBattleBgm,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0x31C4F5F1, // SF1: Wave Battle (SF1 Version)
				0x6AF4E020, // SF2: Wave Battle (SF2 Version)
				0x2C664C1B, // SF3: Ride On (SF3 Version)
			},
			.MenuItem = s_menuItemBossBattleBgm,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0x6046E56E, // SF1: Last Battle (SF1 Version)
				0x11DFAE73, // SF2: Battle Against the Divine
				0x3FDCD876, // SF3: Last Battle (SF3 Version)
			},
			.MenuItem = s_menuItemLastBattleBgm,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0xE007FD11, // SF1: Winner! (SF1 Version)
				0xA7A6A9EC, // SF2: Winner! (SF1 Version)
				0x92B0B3FA, // SF3: Winner! (SF3 Version)
			},
			.MenuItem = s_menuItemWinnerBgm,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0x0DB8091D, // SF1: Loser...
				0x4360DD04, // SF2: Loser...
				0x6ACC1BA0, // SF3: Loser...
			},
			.MenuItem = s_menuItemLoserBgm,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0x3C2A3E79, // SF1: We Have a Problem! (SF1 Version)
				0x6CE164B2, // SF2: We Have a Problem! (SF2 Version)
				0x58890B9D, // SF3: We Have a Problem! (SF3 Version)
			},
			.MenuItem = s_menuItemIncidentBgmRealWorld,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0x5B5B6398, // SF1: Warning Bell! (SF1 Version)
				0x720E5FFA, // SF2: Warning Bell! (SF2 Version)
				0x8ECE5019, // SF3: We Have a Problem! - Wave World (SF3 Version)
			},
			.MenuItem = s_menuItemIncidentBgmWaveWorld,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0xAAF919F9, // SF1: Shooting Star (SF1 Version)
				0x5375B7AF, // SF2: Shooting Star (SF2 Version)
				0xF023375A, // SF3: Shooting Star (SF3 Version)
			},
			.MenuItem = s_menuItemTitleScreenBgm,
		},
		{
			.Type = MusicSettingType::CustomPlaylist,
			.TriggerIDs = {
				0xB73EA2B6, // SF1: FM-ian Theme
				0x6DFDA90A, // SF1: Cheerful Company
				0xEC4B9853, // SF1: Tension (SF1 Version)
				0x1895189D, // SF1: Moving Scene
				0xE041C235, // SF1: Agonizing Wish
				0x8489A114, // SF1: (transformation)
				0xA5C48DF3, // SF1: Winner! - Short (SF1 Version)
				0xD19E7AF5, // SF1: Game Over
				0x11DA3ED0, // SF1: Battle Prelude (SF1 Version)
				0x522A7C35, // SF1: Loneliness
				0x79620121, // SF1: Sweet Frequency
				0x9CB5A790, // SF1: A Place to Return
				0x326F03DC, // SF2: Vega's Theme
				0x4C86D9BA, // SF2: Anthem of the Solitary
				0xBB2D3340, // SF2: My Friends
				0xC1004ADD, // SF2: Cheerful Company
				0x305D0B92, // SF2: Ominous Atmosphere
				0xB166E74D, // SF2: Moving Scene
				0xDD92FCFB, // SF2: Sentimental (SF2 Version)
				0x0C4E4C2D, // SF2: (transformation)
				0x24628ADA, // SF2: Winner! - Short (SF1 Version)
				0x93D25171, // SF2: Game Over
				0x640BD76E, // SF2: Battle Prelude (SF2 Version)
				0xC2ABB29D, // SF2: Loneliness
				0x73B41024, // SF2: Sky-Hi Coliseum
				0xBE23BF31, // SF2: Network
				0x2B0C61C5, // SF2: Eternal Memories
				0xA2DC17E9, // SF3: Satella Police, Move Out!
				0xA86FBCFA, // SF3: Shoot and Slash!
				0xB49CA84A, // SF3: Dealer's Theme
				0x0FE9F0A4, // SF3: Jack and Queen
				0xE1AD6054, // SF3: Anthem of the Solitary
				0xCCAFBBBC, // SF3: Battle Prelude (SF3 Version)
				0x5448F0F6, // SF3: Cheerful Company
				0xA98540F8, // SF3: My Friends
				0xB8F8A027, // SF3: Tension (SF3 Version)
				0x26957CE4, // SF3: Moving Scene
				0xA90C5B1F, // SF3: Sentimental (SF3 Version)
				0x29E0CC15, // SF3: Loneliness
				0xF37BEB7B, // SF3: Purpose Achieved!
				0xBD886180, // SF3: Winner! - Short (SF3 Version)
				0x5A095B0E, // SF3: Game Over
				0x5F2D9E4C, // SF3: Warning Bell! (SF3 Version)
				0x4E461546, // SF3: Noise Change
				0x18CCC173, // SF3: Journey's End
				0x6B21F3F1, // SF3: Chaotic Waves
				0xA3CD607A, // SF3: Shooting Star - Misora Live Version (SF3 Version)
				0x0D65E048, // SF3: Shooting Star - Sonia Live Version (SF3 Version)
			},
			.MenuItem = s_menuItemMiscellaneousBgm,
		},
	};
}
