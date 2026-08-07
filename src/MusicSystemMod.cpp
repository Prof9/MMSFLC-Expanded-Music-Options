#include <Enums_Internal.hpp>

#include <memory>

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

/// @brief Install music system mod after game has launched
void MusicSystemMod::install()
{
	init();

	// Load strings
	MessageManager messageManager(getNativeSingleton("via.gui.MessageManager"));
	messageManager.createAndLoadMessages(MusicSystemMod::STRINGS, via::Language::English);

	buildMenu();

	installHooks();
}

/// @brief Uninstall music system mod
void MusicSystemMod::uninstall()
{
	uninstallHooks();
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
	Object soundMilkyManager = getType("app.sound.SoundMilkyManager")["_Instance"];
	soundMilkyManager.set<bool>("EnableBgmArrange", isArranged);

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

void MusicSystemMod::reloadBgmForMenuItem(const MenuItem *menuItem)
{
	// Find music setting info belonging to this menu item
	const MusicSystemMod::MusicSettingInfo *musicSettingInfo = MusicSystemMod::getMusicSettingInfoForMenuItem(menuItem);
	if (musicSettingInfo == nullptr)
	{
		return;
	}

	Object soundMilkyManager = getType("app.sound.SoundMilkyManager")["_Instance"];
	Object ingameSound = soundMilkyManager["_IngameSoundSystem"];
	if (ingameSound != nullptr)
	{
		// Go through all loaded BGMs
		Object playingBgmInfoList = ingameSound["_PlayingBgmInfoList"];
		std::int32_t playingBgmInfoListLength = playingBgmInfoList.get<std::int32_t>("Length");
		for (std::int32_t i = 0; i < playingBgmInfoListLength; ++i)
		{
			Object playingBgmInfo = playingBgmInfoList[i];

			// Check if this player is overridden by this music setting
			uint32_t playTriggerID = s_playerPlayTriggerID[i];
			auto it = std::ranges::find(musicSettingInfo->TriggerIDs, playTriggerID);
			if (it != musicSettingInfo->TriggerIDs.end())
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
					ingameSound.call("coreStopBgm", ingameSound["_BgmSetting"].get<std::uint32_t>("DefaultStopTriggerID"), i);
					playingBgmInfo.set<std::uint8_t>("State", app::cSound_Base::PlayingBgmStetaEnum::PauseStop);
					break;
				}
			}
		}
	}
}
