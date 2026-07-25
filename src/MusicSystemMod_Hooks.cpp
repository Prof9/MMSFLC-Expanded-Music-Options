#include <Enums_Internal.hpp>

#include <memory>

#include "AudioMenuExtension.hpp"
#include "Common.hpp"
#include "CustomPlaylistMenuItem.hpp"
#include "MusicSystemMod.hpp"
#include "REFrameworkHelper.hpp"

using namespace REFrameworkHelper;

/// @brief Install all hooks used for music system mod
void MusicSystemMod::installHooks()
{
	static bool s_corePlayBgm_skip;
	static void *s_corePlayBgm_retVal;

	// Override corePlayBgm
	s_hooks.emplace_back(hook(
		"app.cSound_Base.corePlayBgm",
		[](int argc, void **argv, auto...)
		{
			static bool isRecursiveCall = false;
			if (isRecursiveCall)
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			Object sound = Object(argv[1]);
			std::uint32_t triggerId = (std::uint32_t)(intptr_t)argv[2];
			Object origId2TriggerId = Object(argv[3]);
			std::uint32_t playerId = (std::uint32_t)(intptr_t)argv[4];

			auto selectNoChange = [&](MusicSettingInfo &musicSettingInfo)
			{
				return false;
			};
			auto selectMusicOff = [&](MusicSettingInfo &musicSettingInfo)
			{
				std::uint32_t stopTriggerId = sound["_BgmSetting"].get<std::uint32_t>("DefaultStopTriggerId");

				sound.call("coreStopBgm", stopTriggerId, playerId);

				s_corePlayBgm_retVal = 0;
				s_corePlayBgm_skip = true;
				return true;
			};
			auto selectAlwaysOriginal = [&](MusicSettingInfo &musicSettingInfo)
			{
				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];

				sound["_Manager"].set<bool>("EnableBgmArrange", false);

				sound["_ResidentContainer"].call(
					"trigger(System.UInt32, via.GameObject, via.GameObject, System.UInt32, System.Boolean, System.UInt32, via.simplewwise.CallbackType, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>)",
					triggerId,
					playerObject,
					nullptr,
					0xFFFFFFFF,
					false,
					0,
					0,
					nullptr,
					nullptr,
					nullptr,
					nullptr);
				playingBgmInfo.set<bool>("IsPlayOriginalContainer", true);
				playingBgmInfo.set<bool>("PlayType", 0);

				s_corePlayBgm_retVal = (void *)getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
				s_corePlayBgm_skip = true;
				return true;
			};
			auto selectAlwaysArranged = [&](MusicSettingInfo &musicSettingInfo)
			{
				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];

				sound["_Manager"].set<bool>("EnableBgmArrange", true);

				sound["_ResidentContainer"].call(
					"trigger(System.UInt32, via.GameObject, via.GameObject, System.UInt32, System.Boolean, System.UInt32, via.simplewwise.CallbackType, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>)",
					triggerId,
					playerObject,
					nullptr,
					0xFFFFFFFF,
					false,
					0,
					0,
					nullptr,
					nullptr,
					nullptr,
					nullptr);
				playingBgmInfo.set<bool>("IsPlayOriginalContainer", true);
				playingBgmInfo.set<bool>("PlayType", 0);

				s_corePlayBgm_retVal = (void *)getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
				s_corePlayBgm_skip = true;
				return true;
			};
			auto selectMix = [&](MusicSettingInfo &musicSettingInfo)
			{
				bool isArranged = sound.get<std::uint8_t>("_CurrentBgmPlayType") == getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Arrange");

				switch ((CustomPlaylistMenuItem::Option)musicSettingInfo.MenuItem->getValue())
				{
				case CustomPlaylistMenuItem::Option::AlwaysOriginal:
					isArranged = false;
					break;
				case CustomPlaylistMenuItem::Option::AlwaysArranged:
					isArranged = true;
					break;
				case CustomPlaylistMenuItem::Option::PreferredMix:
					Object playlist = CustomPlaylistMenuItem::getPreferredMixPlaylist();
					Object musicPlayerBgmDefineList = getType("app.sound.SoundMilkyDefine")["MusicPlayerBgmDefineList"];
					Object bgmSettingDataList = sound["_Manager"]["_BgmSettingDataList"];
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
					uint32_t checkTriggerId = triggerId;
					if (auto it = MAP_REPEAT_TRIGGER_IDS.find(triggerId);
						it != MAP_REPEAT_TRIGGER_IDS.end())
					{
						checkTriggerId = it->second;
					}

					bool found = false;
					std::int32_t playlistLen = playlist.get<std::int32_t>("Count");
					for (std::int32_t i = 0; i < playlistLen; i++)
					{
						Object favMusicInfo = playlist[i];
						std::uint16_t musicId = favMusicInfo.get<std::uint16_t>("musicId");

						std::uint64_t musicPlayerBgmDefineRaw = musicPlayerBgmDefineList.get<std::uint64_t>(musicId);
						std::uint8_t series = *(std::uint8_t *)musicPlayerBgmDefineType_Series->get_data_raw(&musicPlayerBgmDefineRaw, true);
						std::uint8_t bgmSettingIndex = *(std::uint8_t *)musicPlayerBgmDefineType_BgmSettingIndex->get_data_raw(&musicPlayerBgmDefineRaw, true);

						std::uint32_t playerTriggerId = bgmSettingDataList[series]["BgmId2TriggerIdList"][bgmSettingIndex].get<std::uint32_t>("PlayTriggerId");

						if (triggerId == playerTriggerId)
						{
							found = true;
							isArranged = favMusicInfo.get<bool>("isArrange");
						}
					}
					break;
				}

				sound["_Manager"].set<bool>("EnableBgmArrange", isArranged);

				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];
				sound["_ResidentContainer"].call(
					"trigger(System.UInt32, via.GameObject, via.GameObject, System.UInt32, System.Boolean, System.UInt32, via.simplewwise.CallbackType, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>)",
					triggerId,
					playerObject,
					nullptr,
					0xFFFFFFFF,
					false,
					0,
					0,
					nullptr,
					nullptr,
					nullptr,
					nullptr);
				playingBgmInfo.set<bool>("IsPlayOriginalContainer", true);
				playingBgmInfo.set<bool>("PlayType", 0);

				s_corePlayBgm_retVal = (void *)getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
				s_corePlayBgm_skip = true;
				return true;
			};
			auto selectPlaylist = [&](MusicSettingInfo &musicSettingInfo)
			{
				Object playlist;
				bool isCustomPlaylist = (CustomPlaylistMenuItem::Option)musicSettingInfo.MenuItem->getValue() == CustomPlaylistMenuItem::Option::Playlist;
				if (isCustomPlaylist)
				{
					std::shared_ptr<CustomPlaylistMenuItem> playlistMenuItem = std::dynamic_pointer_cast<CustomPlaylistMenuItem>(musicSettingInfo.MenuItem);
					playlist = playlistMenuItem->getCustomPlaylist();
				}
				else
				{
					playlist = getSingleton("app.Launcher")["_saveData"]["favoriteMusicList"];
				}

				if (playlist == nullptr)
				{
					return false;
				}
				std::int32_t playlistSize = playlist.get<std::int32_t>("Count");
				if (playlistSize == 0)
				{
					return false;
				}
				std::int32_t r = getType("via.MathEx").call<std::int32_t>("randomRangeIndex", 0, playlistSize);
				Object musicInfo = playlist[r];
				std::uint16_t musicId = musicInfo.get<std::uint16_t>("musicId");
				bool isArrange = musicInfo.get<bool>("isArrange");

				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];
				if (!MusicSystemMod::playBgm(playerObject, musicId, isArrange))
				{
					return false;
				}

				Type soundMilkyDefine = getType("app.sound.SoundMilkyDefine");
				std::uint8_t series = soundMilkyDefine.call<std::uint8_t>("getMusicPlayerBgmDefine_Series", musicId);
				std::uint8_t bgmSettingIndex = soundMilkyDefine.call<std::uint8_t>("getMusicPlayerBgmDefine_Index", musicId);
				playingBgmInfo.set<std::uint8_t>("FavoriteBgmId_Series", series);
				playingBgmInfo.set<std::uint8_t>("FavoriteBgmId_Index", bgmSettingIndex);
				playingBgmInfo.set<std::uint8_t>("PlayType", getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Favorite"));

				s_corePlayBgm_retVal = (void *)getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Favorite");
				s_corePlayBgm_skip = true;
				return true;
			};
			auto selectDlcMusic = [&](MusicSettingInfo &musicSettingInfo)
			{
				Object extraResource01 = sound
					["_Manager"]["_ExtraResourceList"]
					[app::sound::SoundMilkyManager::ExtraResourceTypeEnum::ExtraResource01];
				if (extraResource01 == nullptr || (Object)(extraResource01["GameObject"]) == nullptr)
				{
					return false;
				}
				Object container = extraResource01["Container"];
				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];

				container.call(
					"trigger(System.UInt32, via.GameObject, via.GameObject, System.UInt32, System.Boolean, System.UInt32, via.simplewwise.CallbackType, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>)",
					musicSettingInfo.NewTriggerID,
					playerObject,
					nullptr,
					0xFFFFFFFF,
					false,
					0,
					0,
					nullptr,
					nullptr,
					nullptr,
					nullptr);

				s_corePlayBgm_retVal = (void *)getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
				s_corePlayBgm_skip = true;
				return true;
			};

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

				bool replaced = false;
				if (std::find(
						musicSettingInfo.TriggerIDs.begin(),
						musicSettingInfo.TriggerIDs.end(),
						triggerId) != musicSettingInfo.TriggerIDs.end())
				{
					switch (musicSettingInfo.Type)
					{
					case MusicSettingType::CustomPlaylist:
						switch ((CustomPlaylistMenuItem::Option)musicSettingInfo.MenuItem->getValue())
						{
						default:
							replaced = selectNoChange(musicSettingInfo);
							break;
						case CustomPlaylistMenuItem::Option::MusicOff:
							replaced = selectMusicOff(musicSettingInfo);
							break;
						case CustomPlaylistMenuItem::Option::AlwaysOriginal:
						case CustomPlaylistMenuItem::Option::AlwaysArranged:
						case CustomPlaylistMenuItem::Option::PreferredMix:
							replaced = selectMix(musicSettingInfo);
							break;
						case CustomPlaylistMenuItem::Option::Playlist:
						case CustomPlaylistMenuItem::Option::Favorites:
							replaced = selectPlaylist(musicSettingInfo);
							break;
						}
						break;
					case MusicSettingType::DlcMusicReplace:
						if (musicSettingInfo.MenuItem->getValue() != 0)
						{
							replaced = selectDlcMusic(musicSettingInfo);
						}
						break;
					}
				}

				if (replaced)
				{
					return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
				}
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		[](void **retval, auto...)
		{
			if (s_corePlayBgm_skip)
			{
				s_corePlayBgm_skip = false;
				*retval = s_corePlayBgm_retVal;
			}
		}));

	static bool s_disableEnableBgmArrangeHook = false;
	s_hooks.emplace_back(hook(
		"app.sound.SoundMilkyManager.set_EnableBgmArrange(System.Boolean)",
		[](int argc, void **argv, auto...)
		{
			Object manager = Object(argv[1]);
			Object sound = manager["_IngameSoundSystem"];
			if (sound != nullptr && !s_disableEnableBgmArrangeHook)
			{
				bool isArranged = (std::uint8_t)(intptr_t)argv[2];

				std::uint32_t playerId = sound.get<std::uint32_t>("_CurrentPlayBgmPlayerId");
				if (playerId >= s_isPlayerArranged.size())
				{
					s_isPlayerArranged.resize(playerId + 1);
				}
				s_isPlayerArranged[playerId] = isArranged;
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	s_hooks.emplace_back(hook(
		"app.cSound_Base.updateBgmPlayType",
		[](int argc, void **argv, auto...)
		{
			Object sound = Object(argv[1]);
			std::uint32_t playerId = sound.get<std::uint32_t>("_CurrentPlayBgmPlayerId");
			bool isArranged = playerId < s_isPlayerArranged.size() ? s_isPlayerArranged[playerId] : false;

			s_disableEnableBgmArrangeHook = true;
			sound["_Manager"].set<bool>("EnableBgmArrange", isArranged);
			s_disableEnableBgmArrangeHook = false;

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	static std::uint16_t launcherReplacedBgmId = getType("app.sound.SoundMilkyDefine").get<std::uint16_t>("INVALID_BGM_ID");
	s_hooks.emplace_back(hook(
		"app.sound.SoundLauncherBgmManager.update",
		[](int argc, void **argv, auto...)
		{
			Object soundLauncherBgmManager = Object(argv[1]);

			if (!soundLauncherBgmManager.get<bool>("_IsInit"))
			{
				launcherReplacedBgmId = getType("app.sound.SoundMilkyDefine").get<std::uint16_t>("INVALID_BGM_ID");
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	s_hooks.emplace_back(hook(
		"app.sound.SoundLauncherBgmManager.playBgm",
		[](int argc, void **argv, auto...)
		{
			Object soundLauncherBgmManager = Object(argv[1]);
			std::uint16_t bgmId = (std::uint16_t)(intptr_t)argv[2];
			Object srcObj = Object(argv[3]);

			Type soundMilkyDefine = getType("app.sound.SoundMilkyDefine");

			auto selectNoChange = [&](CustomPlaylistMenuItem::Option optionValue)
			{
				return false;
			};
			auto selectMusicOff = [&](CustomPlaylistMenuItem::Option optionValue)
			{
				soundLauncherBgmManager.call("stopMainBgm");

				Type bgmRequestType = getType("app.sound.SoundLauncherBgmManager.BgmRequest");
				std::uint64_t bgmRequestRaw = soundLauncherBgmManager.get<std::uint64_t>("_BgmRequest");
				*(std::uint16_t *)bgmRequestType.m_type->find_field("BgmId")->get_data_raw(&bgmRequestRaw, true) = soundMilkyDefine.get<std::uint16_t>("INVALID_BGM_ID");
				soundLauncherBgmManager.set<std::uint64_t>("_BgmRequest", bgmRequestRaw);

				launcherReplacedBgmId = soundMilkyDefine.get<std::uint16_t>("INVALID_BGM_ID");

				return true;
			};
			auto selectPlaylist = [&](CustomPlaylistMenuItem::Option optionValue)
			{
				Object playlist;
				bool isCustomPlaylist = optionValue == CustomPlaylistMenuItem::Option::Playlist;
				if (isCustomPlaylist)
				{
					playlist = s_menuItemMainMenuBgm->getCustomPlaylist();
				}
				else
				{
					playlist = getSingleton("app.Launcher")["_saveData"]["favoriteMusicList"];
				}

				if (playlist == nullptr)
				{
					return false;
				}
				std::int32_t playlistSize = playlist.get<std::int32_t>("Count");
				if (playlistSize == 0)
				{
					return false;
				}
				std::int32_t r = getType("via.MathEx").call<std::int32_t>("randomRangeIndex", 0, playlistSize);
				Object musicInfo = playlist[r];
				std::uint16_t musicId = musicInfo.get<std::uint16_t>("musicId");
				bool isArrange = musicInfo.get<bool>("isArrange");

				if (!MusicSystemMod::playBgm(srcObj, musicId, isArrange))
				{
					return false;
				}

				// Type bgmRequestType = getType("app.sound.SoundLauncherBgmManager.BgmRequest");
				// std::uint64_t bgmRequestRaw = soundLauncherBgmManager.get<std::uint64_t>("_BgmRequest");
				// *(std::uint16_t *)bgmRequestType.m_type->find_field("BgmId")->get_data_raw(&bgmRequestRaw, true) = musicId;
				// soundLauncherBgmManager.set<std::uint64_t>("_BgmRequest", bgmRequestRaw);

				launcherReplacedBgmId = musicId;
				return true;
			};

			if (srcObj != soundLauncherBgmManager["GameObject"])
			{
				// ignore replace for sub player
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			launcherReplacedBgmId = soundMilkyDefine.get<std::uint16_t>("INVALID_BGM_ID");

			if (bgmId != soundMilkyDefine.get<std::uint16_t>("BGM_LAUNCHER_01") &&
				bgmId != soundMilkyDefine.get<std::uint16_t>("BGM_LAUNCHER_02") &&
				bgmId != soundMilkyDefine.get<std::uint16_t>("BGM_LAUNCHER_03") &&
				bgmId != soundMilkyDefine.get<std::uint16_t>("BGM_LAUNCHER_04"))
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			bool replaced;
			CustomPlaylistMenuItem::Option optionValue = (CustomPlaylistMenuItem::Option)s_menuItemMainMenuBgm->getValue();
			switch (optionValue)
			{
			default:
				replaced = selectNoChange(optionValue);
				break;
			case CustomPlaylistMenuItem::Option::MusicOff:
				replaced = selectMusicOff(optionValue);
				break;
			case CustomPlaylistMenuItem::Option::Playlist:
			case CustomPlaylistMenuItem::Option::Favorites:
				replaced = selectPlaylist(optionValue);
				break;
			}

			if (replaced)
			{
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}
			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	s_hooks.emplace_back(hook(
		"app.sound.SoundLauncherBgmManager.stopBgm",
		[](int argc, void **argv, auto...)
		{
			Object soundLauncherBgmManager = Object(argv[1]);
			// std::uint16_t bgmId = (std::uint16_t)(intptr_t)argv[2];
			Object srcObj = Object(argv[3]);
			// bool isStaffRoll = (bool)(intptr_t)argv[4];

			if (srcObj != soundLauncherBgmManager["GameObject"])
			{
				// ignore replace for sub player
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			if (launcherReplacedBgmId == getType("app.sound.SoundMilkyDefine").get<std::uint16_t>("INVALID_BGM_ID"))
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			// Stop the actual BGM that we're playing
			argv[2] = (void *)(intptr_t)launcherReplacedBgmId;
			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));
}

/// @brief Uninstall all hooks used for music system mod
void MusicSystemMod::uninstallHooks()
{
	for (HookRef &hook : s_hooks)
	{
		hook.unhook();
	}
	s_hooks.clear();
}
