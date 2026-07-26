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

/// @brief Get container and trigger ID for playing given BGM
/// @param bgmId BGM ID
/// @return Container and trigger ID, if BGM is valid
std::optional<std::pair<Object, std::uint32_t>> MusicSystemMod::getBgmContainerAndTriggerId(std::uint16_t bgmId)
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

	std::uint8_t series = soundMilkyDefine.call<std::uint8_t>("getMusicPlayerBgmDefine_Series", bgmId);
	std::uint8_t bgmSettingIndex = soundMilkyDefine.call<std::uint8_t>("getMusicPlayerBgmDefine_Index", bgmId);

	std::uint32_t triggerId;
	Object container;
	switch ((BgmSeries)series)
	{
	case BgmSeries::MMSF1:
	case BgmSeries::MMSF2:
	case BgmSeries::MMSF3:
		triggerId = soundMilkyManager["_BgmSettingDataList"][series]["BgmId2TriggerIdList"][bgmSettingIndex].get<std::uint32_t>("PlayTriggerId");
		container = soundMilkyManager["_Container"];
		break;
	case BgmSeries::MMSFLC:
		triggerId = soundMilkyManager["_LauncherSettingData"]["IF_PlayBgmTriggerIdList"].get<std::uint32_t>(bgmSettingIndex);
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
		triggerId = extraResource["SettingData"]["IF_PlayTriggerIdList"].get<std::uint32_t>(bgmSettingIndex);
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
		triggerId = extraResource["SettingData"]["IF_PlayTriggerIdList"].get<std::uint32_t>(bgmSettingIndex);
		container = extraResource["_Container"];
		break;
	}
	default:
		return {};
	}

	return std::pair<Object, std::uint32_t>{container, triggerId};
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
	auto containerAndTriggerId = getBgmContainerAndTriggerId(bgmId);
	if (!containerAndTriggerId.has_value())
	{
		return false;
	}

	Object container = containerAndTriggerId.value().first;
	std::uint32_t triggerId = containerAndTriggerId.value().second;

	return playBgm(srcObj, bgmId, isArranged, container, triggerId);
}
