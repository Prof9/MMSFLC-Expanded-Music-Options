#pragma once

#include <Enums_Internal.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AudioMenuExtension.hpp"
#include "CustomPlaylistMenuItem.hpp"
#include "Guid.hpp"
#include "MenuItem.hpp"
#include "REFrameworkHelper.hpp"

class MusicSystemMod
{
public:
	enum struct MusicSettingType
	{
		CustomPlaylist,
		ReplaceMusic,
	};

	struct BgmTriggerInfo
	{
		REFrameworkHelper::Object Container;
		std::uint32_t PlayTriggerID;
		std::uint32_t StopTriggerID;
		std::uint32_t FadeOutTriggerID;
		std::uint32_t PauseTriggerID;
		std::uint32_t ResumeTriggerID;
	};

	struct MusicSettingInfo
	{
		MusicSettingType Type;
		std::vector<std::uint32_t> TriggerIDs;
		std::shared_ptr<MenuItem> MenuItem;

		std::uint32_t NewTriggerID = 0xFFFFFFFF;
		app::DLCContentsManager::DLC_TYPE RequiredDLC = app::DLCContentsManager::DLC_TYPE::INVALID;
	};

	struct Settings
	{
		std::int32_t RealWorldBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t WaveRoadBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t WaveSpaceBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t DungeonBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t NormalBattleBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t BossBattleBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t LastBattleBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t WinnerBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t LoserBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t IncidentBgmRealWorld = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t IncidentBgmWaveWorld = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t MiscellaneousBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t TitleScreenBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t MainMenuBgm = std::to_underlying(CustomPlaylistMenuItem::Option::NoChange);
		std::int32_t DlcBgmLoneliness = false;
		std::int32_t DlcBgmAstroWave = false;
		std::int32_t DlcBgmAnthemOfTheSolitary = false;
		std::int32_t DlcBgmCheerfulIndoors = false;
	};

	static void init();

	static void install();
	static void uninstall();
	static void runInBackground();

	static std::optional<BgmTriggerInfo> getBgmTriggerInfo(std::uint16_t bgmId);
	static bool playBgm(REFrameworkHelper::Object srcObj, std::uint16_t bgmId, bool isArranged, REFrameworkHelper::Object container, std::uint32_t triggerId);
	static bool playBgm(REFrameworkHelper::Object srcObj, std::uint16_t bgmId, bool isArranged);

	static void reloadBgmForMenuItem(const MenuItem *menuItem, bool restartBgm);

	static bool hasMainMenuBgm();

private:
	static void installHooks();
	static void uninstallHooks();

	static void buildMenu();

	static void loadSettings();
	static void saveSettings();

	static const MusicSettingInfo *getMusicSettingInfoForMenuItem(const MenuItem *menuItem);
	static std::optional<bool> getOverrideMixForTriggerID(std::uint32_t playTriggerID);
	static void setEnableBgmArrangeForced(bool isArranged);

	// Constants
	static const std::map<Guid, std::map<via::Language, char16_t const *>> STRINGS_ADDED;
	static const std::map<std::string_view, std::map<via::Language, char16_t const *>> STRINGS_REPLACED;
	static const std::vector<MenuItem::Option> OPTIONS_COMMON;
	static const std::vector<MenuItem::Option> OPTIONS_NORMAL_BATTLE;
	static const std::vector<MenuItem::Option> OPTIONS_WINNER_LOSER;
	static const std::vector<MenuItem::Option> OPTIONS_MAIN_MENU;
	static const std::vector<MenuItem::Option> OPTIONS_MAIN_MENU_RESTRICTED;
	static const std::vector<MenuItem::Option> OPTIONS_DLC_BGM;

	// Static state
	static inline bool s_initialized = false;
	static inline std::vector<REFrameworkHelper::HookRef> s_hooks;

	static inline std::unordered_map<std::size_t, bool> s_isPlayerArranged;
	static inline std::unordered_map<std::size_t, std::uint32_t> s_playerPlayTriggerID;
	static inline std::unordered_map<std::size_t, bool> s_isPlayerOverridden;
	static inline std::unordered_map<std::size_t, bool> s_forceUpdatePlayTypeArrange;
	static inline std::unordered_map<std::size_t, bool> s_suppressUpdatePlayType;

	static inline bool s_disableEnableBgmArrangeHook = false;
	static inline Settings s_settings;

	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemRealWorldBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemWaveRoadBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemWaveSpaceBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemDungeonBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemNormalBattleBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemBossBattleBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemLastBattleBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemWinnerBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemLoserBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemIncidentBgmRealWorld;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemIncidentBgmWaveWorld;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemMiscellaneousBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemTitleScreenBgm;
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_menuItemMainMenuBgm;
	static inline std::shared_ptr<MenuItem> s_menuItemDlcBgmLoneliness;
	static inline std::shared_ptr<MenuItem> s_menuItemDlcBgmAstroWave;
	static inline std::shared_ptr<MenuItem> s_menuItemDlcBgmAnthemOfTheSolitary;
	static inline std::shared_ptr<MenuItem> s_menuItemDlcBgmCheerfulIndoors;

	static inline std::vector<MusicSettingInfo> s_musicSettingInfoList;
	static inline std::unique_ptr<AudioMenuExtension> s_audioMenuExtension;
};
