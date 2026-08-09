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
	// Override corePlayBgm
	static std::optional<std::uint8_t> s_corePlayBgm_retVal;
	s_hooks.emplace_back(hook(
		"app.cSound_Base.corePlayBgm",
		[](int argc, void **argv, auto...)
		{
			Object sound = Object(argv[1]);
			std::uint32_t triggerId = (std::uint32_t)(intptr_t)argv[2];
			Object origId2TriggerId = Object(argv[3]);
			std::uint32_t playerId = (std::uint32_t)(intptr_t)argv[4];

			bool isEmptyPreferredMix = false;
			bool isArranged = sound.get<std::uint8_t>("_CurrentBgmPlayType") == getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Arrange");

			auto selectNoChange = [&](MusicSettingInfo &musicSettingInfo) -> std::optional<std::uint8_t>
			{
				return {};
			};
			auto selectMusicOff = [&](MusicSettingInfo &musicSettingInfo) -> std::optional<std::uint8_t>
			{
				std::uint32_t stopTriggerId = sound["_BgmSetting"].get<std::uint32_t>("DefaultStopTriggerId");

				sound.call("coreStopBgm", stopTriggerId, playerId);

				return 0;
			};
			auto selectAlwaysOriginal = [&](MusicSettingInfo &musicSettingInfo) -> std::optional<std::uint8_t>
			{
				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];

				MusicSystemMod::setEnableBgmArrangeForced(false);

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

				return getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
			};
			auto selectAlwaysArranged = [&](MusicSettingInfo &musicSettingInfo) -> std::optional<std::uint8_t>
			{
				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];

				MusicSystemMod::setEnableBgmArrangeForced(true);

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

				return getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
			};
			auto selectMix = [&](MusicSettingInfo &musicSettingInfo) -> std::optional<std::uint8_t>
			{
				bool isPreferredMix = (CustomPlaylistMenuItem::Option)musicSettingInfo.MenuItem->getValue() == CustomPlaylistMenuItem::Option::PreferredMix;

				// This will internally loop through the music settings info list again...
				std::optional<bool> mixOverride = MusicSystemMod::getOverrideMixForTriggerID(triggerId);

				// We always need to set EnableBgmArrange appropriately here
				if (mixOverride.has_value())
				{
					isArranged = mixOverride.value();
				}

				if (isPreferredMix && !mixOverride.has_value())
				{
					// We don't want this player to be overwritten; it should follow the BGM Selection setting
					isEmptyPreferredMix = true;
				}

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

				return getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
			};
			auto selectPlaylist = [&](MusicSettingInfo &musicSettingInfo) -> std::optional<std::uint8_t>
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
					return {};
				}
				std::int32_t playlistSize = playlist.get<std::int32_t>("Count");
				if (playlistSize == 0)
				{
					return {};
				}
				std::int32_t r = getType("via.MathEx").call<std::int32_t>("randomRangeIndex", 0, playlistSize);
				Object musicInfo = playlist[r];
				std::uint16_t musicId = musicInfo.get<std::uint16_t>("musicId");
				isArranged = musicInfo.get<bool>("isArrange");

				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];
				if (!MusicSystemMod::playBgm(playerObject, musicId, isArranged))
				{
					return {};
				}

				Type soundMilkyDefine = getType("app.sound.SoundMilkyDefine");
				std::uint8_t series = soundMilkyDefine.call<std::uint8_t>("getMusicPlayerBgmDefine_Series", musicId);
				std::uint8_t bgmSettingIndex = soundMilkyDefine.call<std::uint8_t>("getMusicPlayerBgmDefine_Index", musicId);
				playingBgmInfo.set<std::uint8_t>("FavoriteBgmId_Series", series);
				playingBgmInfo.set<std::uint8_t>("FavoriteBgmId_Index", bgmSettingIndex);
				playingBgmInfo.set<std::uint8_t>("PlayType", getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Favorite"));

				return getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Favorite");
			};
			auto selectDlcMusic = [&](MusicSettingInfo &musicSettingInfo) -> std::optional<std::uint8_t>
			{
				Object extraResource01 = sound
					["_Manager"]["_ExtraResourceList"]
					[app::sound::SoundMilkyManager::ExtraResourceTypeEnum::ExtraResource01];
				if (extraResource01 == nullptr || (Object)(extraResource01["GameObject"]) == nullptr)
				{
					return {};
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

				return getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
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

				if (std::ranges::find(musicSettingInfo.TriggerIDs, triggerId) != musicSettingInfo.TriggerIDs.end())
				{
					switch (musicSettingInfo.Type)
					{
					case MusicSettingType::CustomPlaylist:
						switch ((CustomPlaylistMenuItem::Option)musicSettingInfo.MenuItem->getValue())
						{
						default:
							s_corePlayBgm_retVal = selectNoChange(musicSettingInfo);
							break;
						case CustomPlaylistMenuItem::Option::MusicOff:
							s_corePlayBgm_retVal = selectMusicOff(musicSettingInfo);
							break;
						case CustomPlaylistMenuItem::Option::AlwaysOriginal:
						case CustomPlaylistMenuItem::Option::AlwaysArranged:
						case CustomPlaylistMenuItem::Option::PreferredMix:
							s_corePlayBgm_retVal = selectMix(musicSettingInfo);
							break;
						case CustomPlaylistMenuItem::Option::Playlist:
						case CustomPlaylistMenuItem::Option::Favorites:
							s_corePlayBgm_retVal = selectPlaylist(musicSettingInfo);
							break;
						}
						break;
					case MusicSettingType::ReplaceMusic:
						if (musicSettingInfo.MenuItem->getValue() != 0)
						{
							s_corePlayBgm_retVal = selectDlcMusic(musicSettingInfo);
						}
						break;
					}
				}

				if (s_corePlayBgm_retVal.has_value())
				{
					break;
				}
			}

			// For empty Preferred Mix, we want to have the player not be overridden (it should follow Arrange flag)
			// However we do want to block music restart, since we will play the original track over MMBN tracks
			MusicSystemMod::setEnableBgmArrangeForced(isArranged);
			s_playerPlayTriggerID[playerId] = triggerId;
			s_isPlayerOverridden[playerId] = isEmptyPreferredMix ? false : s_corePlayBgm_retVal.has_value();
			s_suppressUpdatePlayType[playerId] = s_corePlayBgm_retVal.has_value();
			return s_corePlayBgm_retVal.has_value() ? REFRAMEWORK_HOOK_SKIP_ORIGINAL : REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		[](void **retval, auto...)
		{
			if (s_corePlayBgm_retVal.has_value())
			{
				*retval = (void *)std::move(*s_corePlayBgm_retVal);
				s_corePlayBgm_retVal.reset();
			}
		}));

	s_hooks.emplace_back(hook(
		"app.sound.SoundMilkyManager.set_EnableBgmArrange(System.Boolean)",
		[](int argc, void **argv, auto...)
		{
			Object manager = Object(argv[1]);
			Object sound = manager["_IngameSoundSystem"];
			if (sound != nullptr)
			{
				// Call normally if we are in music player
				if (!getSingleton("app.Launcher").call<bool>("isMusicPlayerEnd"))
				{
					return REFRAMEWORK_HOOK_CALL_ORIGINAL;
				}

				std::uint32_t playerId = sound.get<std::uint32_t>("_CurrentPlayBgmPlayerId");
				if (s_disableEnableBgmArrangeHook)
				{
					bool isArranged = (std::uint8_t)(intptr_t)argv[2];
					s_isPlayerArranged[playerId] = isArranged;
				}
				else if (s_isPlayerOverridden[playerId])
				{
					return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
				}
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	s_hooks.emplace_back(hook(
		"app.cSound_Base.doUpdate",
		[](auto...)
		{
			// Skip ingame sound update if we are in music player
			if (!getSingleton("app.Launcher").call<bool>("isMusicPlayerEnd"))
			{
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	static std::vector<app::cSound_Base::PlayingBgmStetaEnum> updateBgmPlayType_playingBgmInfoStates;
	static Object updateBgmPlayType_sound;
	s_hooks.emplace_back(hook(
		"app.cSound_Base.updateBgmPlayType",
		[](int argc, void **argv, auto...)
		{
			Object sound = Object(argv[1]);
			updateBgmPlayType_sound = sound;

			std::uint32_t playerId = sound.get<std::uint32_t>("_CurrentPlayBgmPlayerId");
			if (s_isPlayerOverridden[playerId] || s_forceUpdatePlayTypeArrange[playerId])
			{
				// Set BGM arrange flag to what we know it to be
				MusicSystemMod::setEnableBgmArrangeForced(s_isPlayerArranged[playerId]);
				s_forceUpdatePlayTypeArrange[playerId] = false;
			}
			else
			{
				// Allow updateBgmPlayType to update Arranged flag
				s_disableEnableBgmArrangeHook = true;
			}

			// Set playing BGM state to None temporarily for our overridden players
			// This way they will be skipped by updateBgmPlayType
			Object playingBgmInfoList = sound["_PlayingBgmInfoList"];
			std::int32_t playingBgmInfoListLength = playingBgmInfoList.get<std::int32_t>("Length");
			updateBgmPlayType_playingBgmInfoStates.resize(playingBgmInfoListLength);
			for (std::int32_t i = 0; i < playingBgmInfoListLength; ++i)
			{
				Object playingBgmInfo = playingBgmInfoList[i];
				app::cSound_Base::PlayingBgmStetaEnum bgmState = (app::cSound_Base::PlayingBgmStetaEnum)playingBgmInfo.get<std::int8_t>("State");
				updateBgmPlayType_playingBgmInfoStates[i] = bgmState;

				if (s_suppressUpdatePlayType[i])
				{
					playingBgmInfo.set<std::int8_t>("State", app::cSound_Base::PlayingBgmStetaEnum::None);
				}
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		[](auto...)
		{
			Object sound = updateBgmPlayType_sound;

			s_disableEnableBgmArrangeHook = false;

			// Restore original playing BGM states
			Object playingBgmInfoList = sound["_PlayingBgmInfoList"];
			std::int32_t playingBgmInfoListLength = playingBgmInfoList.get<std::int32_t>("Length");
			for (std::int32_t i = 0; i < playingBgmInfoListLength; ++i)
			{
				Object playingBgmInfo = playingBgmInfoList[i];
				app::cSound_Base::PlayingBgmStetaEnum oldBgmState = updateBgmPlayType_playingBgmInfoStates[i];
				app::cSound_Base::PlayingBgmStetaEnum newBgmState = (app::cSound_Base::PlayingBgmStetaEnum)playingBgmInfo.get<std::int8_t>("State");
				if (newBgmState == app::cSound_Base::PlayingBgmStetaEnum::None)
				{
					playingBgmInfo.set<std::int8_t>("State", oldBgmState);
				}
			}
		}));

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
				if (!MusicSystemMod::hasMainMenuBgm())
				{
					return false;
				}

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

	s_hooks.emplace_back(hook(
		"app.Launcher.setStandbyFlagLauncherOnlyGUIPrefabs",
		[](int argc, void **argv, auto...)
		{
			// Always keep Launcher GUI prefabs loaded
			// Allows loading Music Player while in-game
			argv[2] = (void *)(intptr_t)true;

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	static bool isGamePause = false;
	s_hooks.emplace_back(hook(
		"app.GUILauncherMusicPlayer.update",
		[](auto...)
		{
			// set app.Launcher._isGamePause to false during app.GUILauncherMusicPlayer.update
			// Allows Music Player to work in-game
			Object launcher = getSingleton("app.Launcher");
			isGamePause = launcher.get<bool>("_isGamePause");
			launcher.set<bool>("_isGamePause", false);

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		[](auto...)
		{
			// restore original value of app.Launcher._isGamePause
			Object launcher = getSingleton("app.Launcher");
			launcher.set<bool>("_isGamePause", isGamePause);
		}));

	static std::optional<bool> s_playMusicPlayerBgmReturnValue;
	s_hooks.emplace_back(hook(
		"app.sound.SoundMilkyManager.playMusicPlayerBgm",
		[](int argc, void **argv, auto...)
		{
			Object soundMilkyManager = argv[1];
			std::int32_t bgmId = (std::int32_t)(intptr_t)argv[2];

			switch ((app::AppDefine::GameType)getSingleton("app.Launcher").get<std::int32_t>("gameType"))
			{
			case app::AppDefine::GameType::rr1:
			case app::AppDefine::GameType::rr2:
			case app::AppDefine::GameType::rr3:
				std::optional<BgmTriggerInfo> bgmTriggerInfo = getBgmTriggerInfo(bgmId);
				if (!bgmTriggerInfo.has_value())
				{
					return REFRAMEWORK_HOOK_CALL_ORIGINAL;
				}
				Object container = bgmTriggerInfo.value().Container;
				std::uint32_t triggerId = bgmTriggerInfo.value().PlayTriggerID;

				soundMilkyManager["_CurrentPlayMpContainer"] = container;
				s_playMusicPlayerBgmReturnValue = soundMilkyManager.call<bool>(
					"playMusicPlayerBgmByTriggerId", triggerId);
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		[](void **retval, auto...)
		{
			if (s_playMusicPlayerBgmReturnValue.has_value())
			{
				*retval = (void *)(intptr_t)s_playMusicPlayerBgmReturnValue.value();
				s_playMusicPlayerBgmReturnValue.reset();
			}
		}));

	s_hooks.emplace_back(hook(
		"app.sound.SoundMilkyManager.stopMusicPlayerBgm",
		[](int argc, void **argv, auto...)
		{
			Object soundMilkyManager = argv[1];

			switch ((app::AppDefine::GameType)getSingleton("app.Launcher").get<std::int32_t>("gameType"))
			{
			case app::AppDefine::GameType::rr1:
			case app::AppDefine::GameType::rr2:
			case app::AppDefine::GameType::rr3:
				// Which BGM we are playing doesn't matter
				std::optional<BgmTriggerInfo> bgmTriggerInfo = getBgmTriggerInfo(0);
				if (!bgmTriggerInfo.has_value())
				{
					return REFRAMEWORK_HOOK_CALL_ORIGINAL;
				}
				Object container = bgmTriggerInfo.value().Container;
				std::uint32_t triggerId = bgmTriggerInfo.value().StopTriggerID;
				Object playerObject = soundMilkyManager["GameObject"];

				container.call(
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
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	s_hooks.emplace_back(hook(
		"app.sound.SoundMilkyManager.fadeOutStopMusicPlayerBgm",
		[](int argc, void **argv, auto...)
		{
			Object soundMilkyManager = argv[1];

			switch ((app::AppDefine::GameType)getSingleton("app.Launcher").get<std::int32_t>("gameType"))
			{
			case app::AppDefine::GameType::rr1:
			case app::AppDefine::GameType::rr2:
			case app::AppDefine::GameType::rr3:
				// Which BGM we are playing doesn't matter
				std::optional<BgmTriggerInfo> bgmTriggerInfo = getBgmTriggerInfo(0);
				if (!bgmTriggerInfo.has_value())
				{
					return REFRAMEWORK_HOOK_CALL_ORIGINAL;
				}
				Object container = bgmTriggerInfo.value().Container;
				std::uint32_t triggerId = bgmTriggerInfo.value().FadeOutTriggerID;
				Object playerObject = soundMilkyManager["GameObject"];

				container.call(
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
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	s_hooks.emplace_back(hook(
		"app.sound.SoundMilkyManager.pauseMusicPlayerBgm",
		[](int argc, void **argv, auto...)
		{
			Object soundMilkyManager = argv[1];

			switch ((app::AppDefine::GameType)getSingleton("app.Launcher").get<std::int32_t>("gameType"))
			{
			case app::AppDefine::GameType::rr1:
			case app::AppDefine::GameType::rr2:
			case app::AppDefine::GameType::rr3:
				// Which BGM we are playing doesn't matter
				std::optional<BgmTriggerInfo> bgmTriggerInfo = getBgmTriggerInfo(0);
				if (!bgmTriggerInfo.has_value())
				{
					return REFRAMEWORK_HOOK_CALL_ORIGINAL;
				}
				Object container = bgmTriggerInfo.value().Container;
				std::uint32_t triggerId = bgmTriggerInfo.value().PauseTriggerID;
				Object playerObject = soundMilkyManager["GameObject"];

				container.call(
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
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	s_hooks.emplace_back(hook(
		"app.sound.SoundMilkyManager.resumeMusicPlayerBgm",
		[](int argc, void **argv, auto...)
		{
			Object soundMilkyManager = argv[1];

			switch ((app::AppDefine::GameType)getSingleton("app.Launcher").get<std::int32_t>("gameType"))
			{
			case app::AppDefine::GameType::rr1:
			case app::AppDefine::GameType::rr2:
			case app::AppDefine::GameType::rr3:
				// Which BGM we are playing doesn't matter
				std::optional<BgmTriggerInfo> bgmTriggerInfo = getBgmTriggerInfo(0);
				if (!bgmTriggerInfo.has_value())
				{
					return REFRAMEWORK_HOOK_CALL_ORIGINAL;
				}
				Object container = bgmTriggerInfo.value().Container;
				std::uint32_t triggerId = bgmTriggerInfo.value().ResumeTriggerID;
				Object playerObject = soundMilkyManager["GameObject"];

				container.call(
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
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

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
