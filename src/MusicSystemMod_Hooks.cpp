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

	static std::vector<bool> s_isPlayerArranged;
	static Object s_currentSound;

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

				sound.call(
					"coreStopBgm",
					{
						(void *)(intptr_t)stopTriggerId,
						(void *)(intptr_t)playerId,
					});

				s_corePlayBgm_retVal = 0;
				s_corePlayBgm_skip = true;
				return true;
			};
			auto selectAlwaysOriginal = [&](MusicSettingInfo &musicSettingInfo)
			{
				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];

				sound["_Manager"].call("set_EnableBgmArrange", {(void *)(intptr_t)false});

				sound["_ResidentContainer"].call(
					"trigger(System.UInt32, via.GameObject, via.GameObject, System.UInt32, System.Boolean, System.UInt32, via.simplewwise.CallbackType, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>)",
					{
						(void *)(intptr_t)triggerId,
						playerObject,
						nullptr,
						(void *)(intptr_t)0xFFFFFFFF,
						(void *)(intptr_t)false,
						(void *)(intptr_t)0,
						(void *)(intptr_t)0,
						nullptr,
						nullptr,
						nullptr,
						nullptr,
					});
				playingBgmInfo.set<bool>("IsPlayOriginalContainer", true);
				playingBgmInfo.set<bool>("PlayType", 0);

				s_corePlayBgm_retVal = (void *)(intptr_t)getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
				s_corePlayBgm_skip = true;
				return true;
			};
			auto selectAlwaysArranged = [&](MusicSettingInfo &musicSettingInfo)
			{
				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];

				sound["_Manager"].call("set_EnableBgmArrange", {(void *)(intptr_t)true});

				sound["_ResidentContainer"].call(
					"trigger(System.UInt32, via.GameObject, via.GameObject, System.UInt32, System.Boolean, System.UInt32, via.simplewwise.CallbackType, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>)",
					{
						(void *)(intptr_t)triggerId,
						playerObject,
						nullptr,
						(void *)(intptr_t)0xFFFFFFFF,
						(void *)(intptr_t)false,
						(void *)(intptr_t)0,
						(void *)(intptr_t)0,
						nullptr,
						nullptr,
						nullptr,
						nullptr,
					});
				playingBgmInfo.set<bool>("IsPlayOriginalContainer", true);
				playingBgmInfo.set<bool>("PlayType", 0);

				s_corePlayBgm_retVal = (void *)(intptr_t)getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
				s_corePlayBgm_skip = true;
				return true;
			};
			auto selectMix = [&](MusicSettingInfo &musicSettingInfo)
			{
				bool isArranged = sound.get<std::uint8_t>("_CurrentBgmPlayType") == getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Arrange");

				switch (musicSettingInfo.MenuItem->getValue())
				{
				case std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysOriginal):
					isArranged = false;
					break;
				case std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysArranged):
					isArranged = true;
					break;
				case std::to_underlying(CustomPlaylistMenuItem::Option::PreferMix):
					Object playlist = CustomPlaylistMenuItem::getPreferMixPlaylist();
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

				sound["_Manager"].call("set_EnableBgmArrange", {(void *)(intptr_t)isArranged});

				Object playingBgmInfo = sound["_PlayingBgmInfoList"][playerId];
				Object playerObject = playingBgmInfo["PlayerObject"];
				sound["_ResidentContainer"].call(
					"trigger(System.UInt32, via.GameObject, via.GameObject, System.UInt32, System.Boolean, System.UInt32, via.simplewwise.CallbackType, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>, System.Action`1<soundlib.SoundManager.RequestInfo>)",
					{
						(void *)(intptr_t)triggerId,
						playerObject,
						nullptr,
						(void *)(intptr_t)0xFFFFFFFF,
						(void *)(intptr_t)false,
						(void *)(intptr_t)0,
						(void *)(intptr_t)0,
						nullptr,
						nullptr,
						nullptr,
						nullptr,
					});
				playingBgmInfo.set<bool>("IsPlayOriginalContainer", true);
				playingBgmInfo.set<bool>("PlayType", 0);

				s_corePlayBgm_retVal = (void *)(intptr_t)getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
				s_corePlayBgm_skip = true;
				return true;
			};
			auto selectPlaylist = [&](MusicSettingInfo &musicSettingInfo)
			{
				std::shared_ptr<CustomPlaylistMenuItem> playlistMenuItem = std::dynamic_pointer_cast<CustomPlaylistMenuItem>(musicSettingInfo.MenuItem);
				std::uint8_t bgmPlayTypeFavorite = sound.get<std::uint8_t>("BgmPlayType_Favorite");

				// Leverage existing function to select random song from playlist
				std::uint8_t bgmPlayType = sound.get<std::uint8_t>("_CurrentBgmPlayType");
				sound.set<std::uint8_t>("_CurrentBgmPlayType", bgmPlayTypeFavorite);

				bool isCustomPlaylist = musicSettingInfo.MenuItem->getValue() == std::to_underlying(CustomPlaylistMenuItem::Option::Playlist);

				Object saveData = getSingleton("app.Launcher")["_saveData"];
				Object favoriteMusicList = saveData["favoriteMusicList"];
				if (isCustomPlaylist)
				{
					saveData["favoriteMusicList"] = playlistMenuItem->getCustomPlaylist();
				}

				isRecursiveCall = true;
				sound.call(
					"corePlayBgm",
					{
						(void *)(intptr_t)triggerId,
						origId2TriggerId,
						(void *)(intptr_t)playerId,
					});
				isRecursiveCall = false;

				sound.set<std::uint8_t>("_CurrentBgmPlayType", bgmPlayType);

				if (isCustomPlaylist)
				{
					saveData["favoriteMusicList"] = favoriteMusicList;
				}

				s_corePlayBgm_retVal = (void *)(intptr_t)getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Favorite");
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
					{
						(void *)(intptr_t)musicSettingInfo.NewTriggerID,
						playerObject,
						nullptr,
						(void *)(intptr_t)0xFFFFFFFF,
						(void *)(intptr_t)false,
						(void *)(intptr_t)0,
						(void *)(intptr_t)0,
						nullptr,
						nullptr,
						nullptr,
						nullptr,
					});

				s_corePlayBgm_retVal = (void *)(intptr_t)getType("app.cSound_Base").get<std::uint8_t>("BgmPlayType_Original");
				s_corePlayBgm_skip = true;
				return true;
			};

			Object dlcContentsManager = getSingleton("app.DLCContentsManager");
			for (MusicSettingInfo &musicSettingInfo : s_musicSettingInfoList)
			{
				if (musicSettingInfo.RequiredDLC != app::DLCContentsManager::DLC_TYPE::INVALID)
				{
					if (!dlcContentsManager.call<bool>("getHasDLC", {(void *)(intptr_t)std::to_underlying(musicSettingInfo.RequiredDLC)}))
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
						switch (musicSettingInfo.MenuItem->getValue())
						{
						default:
							replaced = selectNoChange(musicSettingInfo);
							break;
						case std::to_underlying(CustomPlaylistMenuItem::Option::MusicOff):
							replaced = selectMusicOff(musicSettingInfo);
							break;
						case std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysOriginal):
						case std::to_underlying(CustomPlaylistMenuItem::Option::AlwaysArranged):
						case std::to_underlying(CustomPlaylistMenuItem::Option::PreferMix):
							replaced = selectMix(musicSettingInfo);
							break;
						case std::to_underlying(CustomPlaylistMenuItem::Option::Playlist):
						case std::to_underlying(CustomPlaylistMenuItem::Option::Favorites):
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

	s_hooks.emplace_back(hook(
		"app.cSound_Base.baseInit",
		[](int argc, void **argv, auto...)
		{
			s_currentSound = Object(argv[1]);
			s_isPlayerArranged.resize(0);
			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	s_hooks.emplace_back(hook(
		"app.cSound_Base.baseTerminate",
		nullptr,
		[](auto...)
		{
			s_currentSound = nullptr;
			s_isPlayerArranged.resize(0);
		}));

	static bool s_disableEnableBgmArrangeHook = false;
	s_hooks.emplace_back(hook(
		"app.sound.SoundMilkyManager.set_EnableBgmArrange(System.Boolean)",
		[](int argc, void **argv, auto...)
		{
			if (s_currentSound != nullptr && !s_disableEnableBgmArrangeHook)
			{
				bool isArranged = (std::uint8_t)(intptr_t)argv[2];

				std::uint32_t playerId = s_currentSound.get<std::uint32_t>("_CurrentPlayBgmPlayerId");
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
			sound["_Manager"].call("set_EnableBgmArrange", {(void *)(intptr_t)isArranged});
			s_disableEnableBgmArrangeHook = false;

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
