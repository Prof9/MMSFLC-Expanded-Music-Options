#include <Enums_Internal.hpp>

#include <memory>
#include <optional>

#include "MessageManager.hpp"
#include "MusicSystemMod.hpp"
#include "REFrameworkHelper.hpp"

using namespace REFrameworkHelper;

enum struct BgmSeries
{
	MMSF1 = 0,
	MMSF2 = 1,
	MMSF3 = 2,
	MMSFLC = 3,
	MMSFLC_DLC = 4, // Geo Stelar & Omega-Xis Character Model Pack
	MMBN_DLC = 5,	// Mega Man Battle Network Music Series DLC
};

/// @brief Early initialization of music system which can be done before game has fully launched
void MusicSystemMod::init()
{
	if (s_initialized)
	{
		return;
	}
	s_initialized = true;

	loadSettings();
}

void MusicSystemMod::runInBackground()
{
	// Periodically keep checking for DLC to be loaded
	while (true)
	{
		if (hasMainMenuBgm())
		{
			s_menuItemMainMenuBgm->m_options = &OPTIONS_MAIN_MENU;
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

/// @brief Install music system mod after game has launched
void MusicSystemMod::install()
{
	init();

	// Load strings
	MessageManager messageManager(getNativeSingleton("via.gui.MessageManager"));
	messageManager.createAndLoadMessages(MusicSystemMod::STRINGS_ADDED, via::Language::English);
	for (auto stringReplace : MusicSystemMod::STRINGS_REPLACED)
	{
		messageManager.replaceMessageByName(stringReplace.first, stringReplace.second, via::Language::English);
	}

	buildMenu();

	installHooks();

	std::thread backgroundWorker(runInBackground);
	backgroundWorker.detach();
}

/// @brief Uninstall music system mod
void MusicSystemMod::uninstall()
{
	uninstallHooks();
}

bool MusicSystemMod::hasMainMenuBgm()
{
	// Check if any DLC character is installed
	Object dlcContentsManager = getSingleton("app.DLCContentsManager");
	Object companionSetInfoTable = getSingleton("app.Launcher")["companionSetInfoTable"];
	std::int32_t companionSetInfoTableLength = companionSetInfoTable.get<std::int32_t>("Length");
	for (std::int32_t i = 0; i < companionSetInfoTableLength; i++)
	{
		Object companionSetInfo = companionSetInfoTable[i];
		app::DLCContentsManager::DLC_TYPE dlcType = (app::DLCContentsManager::DLC_TYPE)companionSetInfo.get<std::int32_t>("dlcType");

		if (dlcType == app::DLCContentsManager::DLC_TYPE::INVALID)
		{
			continue;
		}
		if (dlcContentsManager.call<bool>("getHasDLC", dlcType))
		{
			return true;
		}
	}
	return false;
}

/// @brief Get BGM container and trigger info for given BGM
/// @param bgmId BGM ID
/// @return BGM container and trigger info, if BGM is valid
std::optional<MusicSystemMod::BgmTriggerInfo> MusicSystemMod::getBgmTriggerInfo(std::uint16_t bgmId)
{
	Type soundMilkyDefine = getType("app.sound.SoundMilkyDefine");

	std::int32_t arrangeBgmStartId = soundMilkyDefine.get<std::int32_t>("ArrangeBgmStartId");
	if (bgmId >= arrangeBgmStartId)
	{
		bgmId -= arrangeBgmStartId;
	}

	Object musicPlayerBgmDefineList = soundMilkyDefine["MusicPlayerBgmDefineList"];
	if (bgmId >= musicPlayerBgmDefineList.get<std::int32_t>("Count"))
	{
		return {};
	}

	Object soundMilkyManager = getType("app.sound.SoundMilkyManager")["_Instance"];

	BgmSeries series = (BgmSeries)soundMilkyDefine.call<std::uint8_t>("getMusicPlayerBgmDefine_Series", bgmId);
	std::uint8_t bgmSettingIndex = soundMilkyDefine.call<std::uint8_t>("getMusicPlayerBgmDefine_Index", bgmId);

	std::uint32_t playTriggerID = 0xFFFFFFFF;
	Object container;
	switch (series)
	{
	case BgmSeries::MMSF1:
	case BgmSeries::MMSF2:
	case BgmSeries::MMSF3:
		playTriggerID = soundMilkyManager["_BgmSettingDataList"][series]["BgmId2TriggerIdList"][bgmSettingIndex].get<std::uint32_t>("PlayTriggerId");
		container = soundMilkyManager["_Container"];
		break;
	case BgmSeries::MMSFLC:
		playTriggerID = soundMilkyManager["_LauncherSettingData"]["IF_PlayBgmTriggerIdList"].get<std::uint32_t>(bgmSettingIndex);
		container = soundMilkyManager["_Container"];
		break;
	case BgmSeries::MMSFLC_DLC:
	{
		// Check DLC owned and loaded
		if (!getSingleton("app.DLCContentsManager").call<bool>("getHasDLC", app::DLCContentsManager::DLC_TYPE::DLC01))
		{
			return {};
		}
		Object extraResource = soundMilkyManager["_ExtraResourceList"][app::sound::SoundMilkyManager::ExtraResourceTypeEnum::ExtraResource01];
		if (extraResource == nullptr || extraResource["GameObject"] == nullptr)
		{
			return {};
		}
		playTriggerID = extraResource["SettingData"]["IF_PlayTriggerIdList"].get<std::uint32_t>(bgmSettingIndex);
		container = extraResource["_Container"];
		break;
	}
	case BgmSeries::MMBN_DLC:
	{
		if (!getSingleton("app.DLCContentsManager").call<bool>("getHasDLC", app::DLCContentsManager::DLC_TYPE::DLC02))
		{
			return {};
		}
		Object extraResource = soundMilkyManager["_ExtraResourceList"][app::sound::SoundMilkyManager::ExtraResourceTypeEnum::ExtraResource02];
		if (extraResource == nullptr || extraResource["GameObject"] == nullptr)
		{
			return {};
		}
		playTriggerID = extraResource["SettingData"]["IF_PlayTriggerIdList"].get<std::uint32_t>(bgmSettingIndex);
		container = extraResource["_Container"];
		break;
	}
	default:
		return {};
	}

	std::uint32_t stopTriggerID = 0xFFFFFFFF;
	std::uint32_t fadeOutTriggerID = 0xFFFFFFFF;
	std::uint32_t pauseTriggerID = 0xFFFFFFFF;
	std::uint32_t resumeTriggerID = 0xFFFFFFFF;
	switch ((app::AppDefine::GameType)getSingleton("app.Launcher").get<std::int32_t>("gameType"))
	{
	case app::AppDefine::GameType::rr1:
		stopTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF1].get<std::uint32_t>("DefaultStopTriggerId");
		fadeOutTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF1].get<std::uint32_t>("PlayingBgmFadeOutTriggerId");
		pauseTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF1].get<std::uint32_t>("PauseTriggerId");
		resumeTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF1].get<std::uint32_t>("ResumeTriggerId");
		break;
	case app::AppDefine::GameType::rr2:
		stopTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF2].get<std::uint32_t>("DefaultStopTriggerId");
		fadeOutTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF2].get<std::uint32_t>("PlayingBgmFadeOutTriggerId");
		pauseTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF2].get<std::uint32_t>("PauseTriggerId");
		resumeTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF2].get<std::uint32_t>("ResumeTriggerId");
		break;
	case app::AppDefine::GameType::rr3:
		stopTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF3].get<std::uint32_t>("DefaultStopTriggerId");
		fadeOutTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF3].get<std::uint32_t>("PlayingBgmFadeOutTriggerId");
		pauseTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF3].get<std::uint32_t>("PauseTriggerId");
		resumeTriggerID = soundMilkyManager["_BgmSettingDataList"][BgmSeries::MMSF3].get<std::uint32_t>("ResumeTriggerId");
		break;
	}

	return BgmTriggerInfo{
		.Container = container,
		.PlayTriggerID = playTriggerID,
		.StopTriggerID = stopTriggerID,
		.FadeOutTriggerID = fadeOutTriggerID,
		.PauseTriggerID = pauseTriggerID,
		.ResumeTriggerID = resumeTriggerID,
	};
}

/// @brief Play BGM
/// @param srcObj Player object
/// @param musicId BGM ID
/// @param isArranged Whether to play arranged or original mix
/// @param container Container containing BGM
/// @param triggerId Trigger ID to play BGM
/// @return True if BGM started, false if BGM could not be started
bool MusicSystemMod::playBgm(Object srcObj, std::uint16_t bgmId, bool isArranged, Object container, std::uint32_t triggerId)
{
	Type soundMilkyDefine = getType("app.sound.SoundMilkyDefine");

	std::int32_t arrangeBgmStartId = soundMilkyDefine.get<std::int32_t>("ArrangeBgmStartId");
	if (bgmId >= arrangeBgmStartId)
	{
		isArranged = true;
	}

	bool canArranged = soundMilkyDefine.call<std::uint8_t>("getMusicPlayerBgmDefine_EnableArrange", bgmId);

	container.call(
		"trigger(System.UInt32, via.GameObject, via.GameObject, System.UInt32, System.Boolean, System.UInt32, via.simplewwise.CallbackType, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>)",
		triggerId,
		srcObj,
		nullptr,
		0xFFFFFFFF,
		false,
		0,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr);

	if (!canArranged)
	{
		isArranged = false;
	}
	MusicSystemMod::setEnableBgmArrangeForced(isArranged);

	return true;
}

/// @brief Play BGM
/// @param srcObj Player object
/// @param musicId BGM ID
/// @param isArranged Whether to play arranged or original mix
/// @return True if BGM started, false if BGM could not be started
bool MusicSystemMod::playBgm(Object srcObj, std::uint16_t bgmId, bool isArranged)
{
	std::optional<BgmTriggerInfo> bgmTriggerInfo = getBgmTriggerInfo(bgmId);
	if (!bgmTriggerInfo.has_value())
	{
		return false;
	}

	Object container = bgmTriggerInfo.value().Container;
	std::uint32_t triggerId = bgmTriggerInfo.value().PlayTriggerID;

	return playBgm(srcObj, bgmId, isArranged, container, triggerId);
}

const MusicSystemMod::MusicSettingInfo *MusicSystemMod::getMusicSettingInfoForMenuItem(const MenuItem *menuItem)
{
	auto it = std::ranges::find_if(
		s_musicSettingInfoList,
		[menuItem](MusicSettingInfo &info)
		{
			return info.MenuItem.get() == menuItem;
		});

	return it == s_musicSettingInfoList.end() ? nullptr : &*it;
}

std::optional<bool> MusicSystemMod::getOverrideMixForTriggerID(std::uint32_t playTriggerID)
{
	bool foundAlwaysOriginal = false;
	bool foundAlwaysArranged = false;
	bool foundPreferOriginal = false;
	bool foundPreferArranged = false;
	Object soundMilkyManager = getType("app.sound.SoundMilkyManager")["_Instance"];
	Object dlcContentsManager = getSingleton("app.DLCContentsManager");
	for (MusicSettingInfo &musicSettingInfo : s_musicSettingInfoList)
	{
		if (musicSettingInfo.RequiredDLC != app::DLCContentsManager::DLC_TYPE::INVALID)
		{
			if (!dlcContentsManager.call<bool>("getHasDLC", musicSettingInfo.RequiredDLC))
			{
				continue;
			}
		}

		if (std::ranges::find(musicSettingInfo.TriggerIDs, playTriggerID) != musicSettingInfo.TriggerIDs.end())
		{
			switch (musicSettingInfo.Type)
			{
			case MusicSettingType::CustomPlaylist:
				switch ((CustomPlaylistMenuItem::Option)musicSettingInfo.MenuItem->getValue())
				{
				case CustomPlaylistMenuItem::Option::AlwaysOriginal:
					foundAlwaysOriginal = true;
					break;
				case CustomPlaylistMenuItem::Option::AlwaysArranged:
					foundAlwaysArranged = true;
					break;
				case CustomPlaylistMenuItem::Option::PreferredMix:
				{
					Object playlist = CustomPlaylistMenuItem::getPreferredMixPlaylist();
					Object musicPlayerBgmDefineList = getType("app.sound.SoundMilkyDefine")["MusicPlayerBgmDefineList"];
					Object bgmSettingDataList = soundMilkyManager["_BgmSettingDataList"];
					Type musicPlayerBgmDefineType = getType("app.sound.SoundMilkyDefine.MusicPlayerBgmDefine");
					reframework::API::Field *musicPlayerBgmDefineType_Series = musicPlayerBgmDefineType.m_type->find_field("Series");
					reframework::API::Field *musicPlayerBgmDefineType_BgmSettingIndex = musicPlayerBgmDefineType.m_type->find_field("BgmSettingIndex");

					/// Map repeat songs (e.g. Winner! in SF2) to their original trigger IDs
					const static std::unordered_map<std::uint32_t, std::uint32_t> MAP_REPEAT_TRIGGER_IDS = {
						{0x412B3875, 0xE994DBDE}, // SF2: Cheerful Indoors -> SF1
						{0xC1004ADD, 0x6DFDA90A}, // SF2: Cheerful Company -> SF1
						{0xB166E74D, 0x1895189D}, // SF2: Moving Scene -> SF1
						{0xA7A6A9EC, 0xE007FD11}, // SF2: Winner! (SF1 Version) -> SF1
						{0x24628ADA, 0xA5C48DF3}, // SF2: Winner! - Short (SF1 Version) -> SF1
						{0x4360DD04, 0x0DB8091D}, // SF2: Loser -> SF1
						{0x93D25171, 0xD19E7AF5}, // SF2: Game Over -> SF1
						{0xC2ABB29D, 0x522A7C35}, // SF2: Loneliness -> SF1
						{0x295EEC52, 0xE994DBDE}, // SF3: Cheerful Indoors -> SF1
						{0xE1AD6054, 0x4C86D9BA}, // SF3: Anthem of the Solitary -> SF2
						{0x5448F0F6, 0x6DFDA90A}, // SF3: Cheerful Company -> SF1
						{0xA98540F8, 0xBB2D3340}, // SF3: My Friends -> SF2
						{0x26957CE4, 0x1895189D}, // SF3: Moving Scene -> SF1
						{0x5A095B0E, 0xD19E7AF5}, // SF3: Game Over -> SF1
						{0x6ACC1BA0, 0x0DB8091D}, // SF3: Loser... -> SF1
					};
					uint32_t checkTriggerID = playTriggerID;
					if (auto it = MAP_REPEAT_TRIGGER_IDS.find(checkTriggerID);
						it != MAP_REPEAT_TRIGGER_IDS.end())
					{
						checkTriggerID = it->second;
					}

					std::int32_t playlistLen = playlist.get<std::int32_t>("Count");
					for (std::int32_t i = 0; i < playlistLen; ++i)
					{
						Object favMusicInfo = playlist[i];
						std::uint16_t musicId = favMusicInfo.get<std::uint16_t>("musicId");

						std::uint64_t musicPlayerBgmDefineRaw = musicPlayerBgmDefineList.get<std::uint64_t>(musicId);
						std::uint8_t series = *(std::uint8_t *)musicPlayerBgmDefineType_Series->get_data_raw(&musicPlayerBgmDefineRaw, true);
						std::uint8_t bgmSettingIndex = *(std::uint8_t *)musicPlayerBgmDefineType_BgmSettingIndex->get_data_raw(&musicPlayerBgmDefineRaw, true);

						std::uint32_t playerTriggerId = bgmSettingDataList[series]["BgmId2TriggerIdList"][bgmSettingIndex].get<std::uint32_t>("PlayTriggerId");

						if (checkTriggerID == playerTriggerId)
						{
							if (favMusicInfo.get<bool>("isArrange"))
							{
								foundPreferArranged = true;
							}
							else
							{
								foundPreferOriginal = true;
							}
						}
					}
					break;
				}
				default:
					break;
				}
				break;
			default:
				break;
			}
		}
	}

	if (foundAlwaysArranged && !foundAlwaysOriginal ||
		foundPreferArranged && !foundPreferOriginal)
	{
		return true;
	}
	if (foundAlwaysOriginal && !foundAlwaysArranged ||
		foundPreferOriginal && !foundPreferArranged)
	{
		return false;
	}
	if (foundAlwaysOriginal && foundAlwaysArranged ||
		foundPreferOriginal && foundPreferArranged)
	{
		// We equally want original and arranged
		// Choose random 50/50
		std::int32_t r = getType("via.MathEx").call<std::int32_t>("randomRangeIndex", 0, 2);
		return r == 1 ? true : false;
	}

	return {};
}

void MusicSystemMod::setEnableBgmArrangeForced(bool isArranged)
{
	s_disableEnableBgmArrangeHook = true;
	Object soundMilkyManager = getType("app.sound.SoundMilkyManager")["_Instance"];
	soundMilkyManager.set<bool>("EnableBgmArrange", isArranged);
	s_disableEnableBgmArrangeHook = false;
}

void MusicSystemMod::reloadBgmForMenuItem(const MenuItem *menuItem, bool restartBgm)
{
	// Find music setting info belonging to this menu item
	const MusicSystemMod::MusicSettingInfo *musicSettingInfo = MusicSystemMod::getMusicSettingInfoForMenuItem(menuItem);

	Object ingameSound = getType("app.sound.SoundMilkyManager")["_Instance"]["_IngameSoundSystem"];
	Object launcherSound = getType("app.sound.SoundLauncherBgmManager")["_Instance"];
	if (ingameSound != nullptr)
	{
		if (musicSettingInfo == nullptr)
		{
			return;
		}

		// Go through all loaded BGMs
		Object playingBgmInfoList = ingameSound["_PlayingBgmInfoList"];
		std::int32_t playingBgmInfoListLength = playingBgmInfoList.get<std::int32_t>("Length");
		for (std::int32_t i = 0; i < playingBgmInfoListLength; ++i)
		{
			Object playingBgmInfo = playingBgmInfoList[i];

			bool restartBgmForPlayer = restartBgm;

			// Check if this player is overridden by this music setting
			std::uint32_t playTriggerID = s_playerPlayTriggerID[i];
			if (std::ranges::find(musicSettingInfo->TriggerIDs, playTriggerID) != musicSettingInfo->TriggerIDs.end())
			{
				std::optional<bool> overrideMix = getOverrideMixForTriggerID(playTriggerID);

				bool isOverrideActive = !menuItem->isDefaultValue();
				bool isOverrideActiveChanged = false;
				bool isHigherPriorityOverrideActive = false;
				bool isLowerPriorityOverrideActive = false;
				bool foundSelf = false;
				for (MusicSystemMod::MusicSettingInfo &otherMusicSettingInfo : s_musicSettingInfoList)
				{
					if (&otherMusicSettingInfo == musicSettingInfo)
					{
						foundSelf = true;
						continue;
					}
					if (otherMusicSettingInfo.MenuItem != nullptr &&
						!otherMusicSettingInfo.MenuItem->isDefaultValue() &&
						std::ranges::find(otherMusicSettingInfo.TriggerIDs, playTriggerID) != otherMusicSettingInfo.TriggerIDs.end())
					{
						if (foundSelf)
						{
							isLowerPriorityOverrideActive = true;
						}
						else
						{
							isHigherPriorityOverrideActive = true;
							break;
						}
					}
				}
				if (isHigherPriorityOverrideActive)
				{
					// If there is a higher priority override active then we should do nothing
					continue;
				}
				if (!isLowerPriorityOverrideActive)
				{
					// Force a restart if we are playing non-default music and override becomes active
					// or if override becomes inactive and we should be playing default music
					if (isOverrideActive && !s_isPlayerOverridden[i] &&
						!playingBgmInfo.get<bool>("IsPlayOriginalContainer"))
					{
						restartBgmForPlayer = true;
					}
					if (!isOverrideActive && s_isPlayerOverridden[i] &&
						ingameSound.get<std::uint8_t>("_CurrentBgmPlayType") != getStaticField<std::uint8_t>("app.cSound_Base.BgmPlayType_Original") &&
						ingameSound.get<std::uint8_t>("_CurrentBgmPlayType") != getStaticField<std::uint8_t>("app.cSound_Base.BgmPlayType_Arrange"))
					{
						restartBgmForPlayer = true;
					}

					// If we are the only active override we should update the override flag

					if (musicSettingInfo->Type == MusicSettingType::CustomPlaylist &&
						(CustomPlaylistMenuItem::Option)menuItem->getValue() == CustomPlaylistMenuItem::Option::PreferredMix &&
						!overrideMix.has_value())
					{
						s_isPlayerOverridden[i] = false;
					}
					else
					{
						s_isPlayerOverridden[i] = isOverrideActive;
					}
					s_suppressUpdatePlayType[i] = isOverrideActive;
				}

				if (restartBgmForPlayer)
				{
					app::cSound_Base::PlayingBgmStetaEnum bgmState = (app::cSound_Base::PlayingBgmStetaEnum)playingBgmInfo.get<std::uint8_t>("State");
					std::uint16_t bgmId = playingBgmInfo.get<std::uint16_t>("OriginalId");

					// This basically replicates app.cSound_Base.updateBgmPlayType() in part
					switch (bgmState)
					{
					case app::cSound_Base::PlayingBgmStetaEnum::Play:
						ingameSound.call("baseChangeBgm", bgmId, 0, i, -1);
						break;
					case app::cSound_Base::PlayingBgmStetaEnum::Pause:
						// TODO: We could do this not immediately but only upon unpause if the effective music will change
						// That way if you change music type and back while in battle (SF2) it won't restart field music
						ingameSound.call("coreStopBgm", ingameSound["_BgmSetting"].get<std::uint32_t>("DefaultStopTriggerId"), i);
						playingBgmInfo.set<std::uint8_t>("State", app::cSound_Base::PlayingBgmStetaEnum::PauseStop);
						break;
					}
				}
				else
				{
					bool isArranged = ingameSound.get<std::uint8_t>("_CurrentBgmPlayType") == getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Arrange");
					if (overrideMix.has_value())
					{
						isArranged = overrideMix.value();
					}
					else
					{
						isOverrideActive = false;
					}

					s_isPlayerArranged[i] = isArranged;
					s_forceUpdatePlayTypeArrange[i] = true;
					if (playingBgmInfo.get<bool>("IsPlaying"))
					{
						MusicSystemMod::setEnableBgmArrangeForced(isArranged);
					}
				}
			}
		}
	}
	else if (launcherSound != nullptr)
	{
		// Check if this is the Main Menu BGM setting
		// Good enough for now since MusicSystemMod owns this menu item
		// Assume for now we can only access Options when the normal launcher BGM is playing
		if (menuItem == s_menuItemMainMenuBgm.get())
		{
			if (restartBgm)
			{
				// Temporarily unset CurrentMainBgmId to force the play request to go through
				std::uint16_t currentMainBgmId = launcherSound.get<std::uint16_t>("_CurrentMainBgmId");
				launcherSound.set<std::uint16_t>("_CurrentMainBgmId", getStaticField<std::uint16_t>("app.sound.SoundLauncherBgmManager.InvalidBgmId"));
				getSingleton("app.Launcher").call("playLauncherMainBgm");
				launcherSound.set<std::uint16_t>("_CurrentMainBgmId", currentMainBgmId);
			}
		}
	}
}
