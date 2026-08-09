#include <Enums_Internal.hpp>

#include <filesystem>
#include <fstream>
#include <limits>
#include <utility>

#include "CustomPlaylistMenuItem.hpp"
#include "MusicSystemMod.hpp"
#include "REFrameworkHelper.hpp"

using namespace REFrameworkHelper;

Object CustomPlaylistMenuItem::getCustomPlaylist() const
{
	return m_customPlaylist;
}

void CustomPlaylistMenuItem::setCustomPlaylist(Object playlist)
{
	if (m_customPlaylist)
	{
		m_customPlaylist.m_object->release();
	}

	m_customPlaylist = playlist;

	if (m_customPlaylist)
	{
		m_customPlaylist.m_object->add_ref();
	}
}

Object CustomPlaylistMenuItem::getPreferredMixPlaylist()
{
	return s_PreferredMixPlaylist;
}

void CustomPlaylistMenuItem::setPreferredMixPlaylist(Object playlist)
{
	if (s_PreferredMixPlaylist)
	{
		s_PreferredMixPlaylist.m_object->release();
	}

	s_PreferredMixPlaylist = playlist;

	if (s_PreferredMixPlaylist)
	{
		s_PreferredMixPlaylist.m_object->add_ref();
	}
}

Object CustomPlaylistMenuItem::loadFavoritesList(const std::filesystem::path &fileName)
{
	auto &api = reframework::API::get();
	Type musicInfoType = getType("app.GUILauncherMusicPlayer.MusicInfo");

	// Create favorites list
	Object favoritesList = createObject("System.Collections.Generic.List`1<app.GUILauncherMusicPlayer.MusicInfo>");
	favoritesList.call(".ctor()");
	favoritesList.m_object->add_ref();

	// Load music info settings from file
	std::ifstream file;
	file.open(fileName, std::fstream::in | std::fstream::binary);
	if (file)
	{
		// Read type CRC for MusicInfo
		std::uint32_t musicInfoCrc;
		file.read((char *)&musicInfoCrc, sizeof(musicInfoCrc));
		if (!file)
		{
			// Error reading from file, stop loading
			api->log_error("Failed to read from favorites file, file may be corrupt");
			return favoritesList;
		}
		if (musicInfoCrc != musicInfoType.m_type->get_type_info()->get_crc())
		{
			api->log_error("app.GUILauncherMusicPlayer.MusicInfo type has changed! Aborting favorites load!");
			return favoritesList;
		}

		std::uint32_t musicInfoTypeSize = musicInfoType.m_type->get_size();
		std::uint32_t managedObjectTypeSize = musicInfoType.m_type->get_parent_type()->get_size();

		while (file.peek() != EOF)
		{
			// Deserialize into allocated object
			Object musicInfo = createObject("app.GUILauncherMusicPlayer.MusicInfo");
			file.read(
				(char *)((intptr_t)musicInfo.m_object + managedObjectTypeSize),
				musicInfoTypeSize - managedObjectTypeSize);
			if (!file)
			{
				api->log_error("Failed to read from favorites file, file may be corrupt");
				break;
			}

			// If object was deserialized, add to the list
			favoritesList.call("Add", musicInfo);
		}
	}

	return favoritesList;
}

bool CustomPlaylistMenuItem::saveFavoritesList(const std::filesystem::path &fileName, Object favoritesList)
{
	auto &api = reframework::API::get();

	// Create directories for file
	if (fileName.has_parent_path())
	{
		std::filesystem::create_directories(fileName.parent_path());
	}

	Type musicInfoType = getType("app.GUILauncherMusicPlayer.MusicInfo");
	std::uint32_t musicInfoTypeSize = musicInfoType.m_type->get_size();
	std::uint32_t managedObjectTypeSize = musicInfoType.m_type->get_parent_type()->get_size();

	std::ofstream file;
	try
	{
		file.open(fileName, std::fstream::out | std::fstream::binary | std::fstream::trunc);
		if (!file)
		{
			api->log_error("Failed to create favorites file");
			std::filesystem::remove(fileName);
			return false;
		}

		// Write type CRC for MusicInfo
		std::uint32_t musicInfoTypeCrc = getType("app.GUILauncherMusicPlayer.MusicInfo").m_type->get_type_info()->get_crc();
		if (!file.write((char *)&musicInfoTypeCrc, sizeof(musicInfoTypeCrc)))
		{
			api->log_error("Failed to write to favorites file");
			file.close();
			std::filesystem::remove(fileName);
			return false;
		}

		std::int32_t count = favoritesList.get<std::int32_t>("Count");
		for (std::int32_t i = 0; i < count; ++i)
		{
			// Serialize object to file
			Object musicInfo = favoritesList[i];
			file.write(
				(char *)((intptr_t)musicInfo.m_object + managedObjectTypeSize),
				musicInfoTypeSize - managedObjectTypeSize);
			if (!file)
			{
				api->log_error("Failed to write to favorites file");
				file.close();
				std::filesystem::remove(fileName);
				return false;
			}
		}
	}
	catch (...)
	{
		try
		{
			api->log_error("Failed to write to favorites file");
			if (file.is_open())
			{
				file.close();
			}
			std::filesystem::remove(fileName);
		}
		catch (...)
		{
		}
		return false;
	}

	return true;
}

const std::filesystem::path &CustomPlaylistMenuItem::getPreferredMixPlaylistFileName()
{
	return s_PreferredMixPlaylistFileName;
}
void CustomPlaylistMenuItem::setPreferredMixPlaylistFileName(const std::filesystem::path &fileName)
{
	s_PreferredMixPlaylistFileName = fileName;
}

CustomPlaylistMenuItem::CustomPlaylistMenuItem(std::optional<const std::filesystem::path> playlistFileName, Guid nameGuid, Guid descriptionGuid, const std::vector<MenuItem::Option> *options, std::int32_t *valuePtr, std::int32_t defaultValue)
	: MenuItem::MenuItem(nameGuid, descriptionGuid, options, valuePtr, defaultValue), m_customPlaylistFileName(playlistFileName)
{
	// Load original/arranged playlist if this is the first instance
	if (!s_PreferredMixPlaylist)
	{
		setPreferredMixPlaylist(loadFavoritesList(s_PreferredMixPlaylistFileName));
	}

	// Load custom playlist
	if (playlistFileName.has_value())
	{
		setCustomPlaylist(loadFavoritesList(m_customPlaylistFileName.value()));
	}
}

CustomPlaylistMenuItem::~CustomPlaylistMenuItem()
{
	setCustomPlaylist((void *)nullptr);
}

void CustomPlaylistMenuItem::setValue(std::int32_t value) const
{
	CustomPlaylistMenuItem::Option oldValue = (CustomPlaylistMenuItem::Option)getValue();
	MenuItem::setValue(value);
	CustomPlaylistMenuItem::Option newValue = (CustomPlaylistMenuItem::Option)getValue();

	std::vector changeBgmMusicOptions{
		CustomPlaylistMenuItem::Option::MusicOff,
		CustomPlaylistMenuItem::Option::Battle,
		CustomPlaylistMenuItem::Option::Field,
		CustomPlaylistMenuItem::Option::FieldOriginal,
		CustomPlaylistMenuItem::Option::FieldArranged,
	};
	constexpr std::array changeBgmMusicOptionsField{
		CustomPlaylistMenuItem::Option::Field,
		CustomPlaylistMenuItem::Option::FieldOriginal,
		CustomPlaylistMenuItem::Option::FieldArranged,
	};

	// Check if playlist or favorites is empty
	// This doesn't check for DLC but oh well
	/*
	Object playlist = getCustomPlaylist();
	Object favorites = getSingleton("app.Launcher")["_saveData"]["favoriteMusicList"];
	if (playlist.get<std::int32_t>("Count") > 0)
	{
		changeBgmMusicOptions.push_back(CustomPlaylistMenuItem::Option::Playlist);
	}
	if (favorites.get<std::int32_t>("Count") > 0)
	{
		changeBgmMusicOptions.push_back(CustomPlaylistMenuItem::Option::Favorites);
	}
	*/

	// TODO for now just always add them
	// Otherwise going empty playlist -> exit to start MMBN music -> change to mix option doesn't restart music
	changeBgmMusicOptions.push_back(CustomPlaylistMenuItem::Option::Playlist);
	changeBgmMusicOptions.push_back(CustomPlaylistMenuItem::Option::Favorites);

	bool restartBgm = false;
	if (std::ranges::contains(changeBgmMusicOptions, oldValue) ||
		std::ranges::contains(changeBgmMusicOptions, newValue))
	{
		// We are changing to or from an option that can replace music
		restartBgm = true;
	}
	if (std::ranges::contains(changeBgmMusicOptionsField, oldValue) &&
		std::ranges::contains(changeBgmMusicOptionsField, newValue))
	{
		// BGM shouldn't restart in this case
		restartBgm = false;
	}

	MusicSystemMod::reloadBgmForMenuItem(this, restartBgm);
}

bool CustomPlaylistMenuItem::canEnter()
{
	switch (m_state)
	{
	case State::Idle:
	case State::OpeningMusicPlayer:
		// Return true when opening as well
		// Otherwise Edit List disappears from operation guide
		break;
	default:
		return false;
	}

	// Check if the current value is a playlist that can be edited
	switch ((CustomPlaylistMenuItem::Option)getValue())
	{
	case CustomPlaylistMenuItem::Option::PreferredMix:
	case CustomPlaylistMenuItem::Option::Playlist:
	case CustomPlaylistMenuItem::Option::Favorites:
		return true;
	default:
		return false;
	}
}

bool CustomPlaylistMenuItem::onEnter()
{
	if (!this->canEnter())
	{
		return false;
	}

	switch (m_state)
	{
	case State::Idle:
		break;
	default:
		return false;
	}

	Object launcher = getSingleton("app.Launcher");
	launcher.call("showLoadingGUI", app::LauncherFadeUserData::FadeUsage::GalleryToMusicPlayer2Way, false, false);

	m_state = State::OpeningMusicPlayer;
	return true;
}

bool CustomPlaylistMenuItem::onUpdate()
{
	switch (m_state)
	{
	case State::Idle:
		return true;
	case State::OpeningMusicPlayer:
	{
		Object launcher = getSingleton("app.Launcher");
		bool isFadeInAnimEnd = launcher["loadingGuiBehavior"].call<bool>("isFadeInAnimEnd");
		if (isFadeInAnimEnd)
		{
			// set DrawSelf = false
			launcher["guiBehaviors"][app::Launcher::LauncherGUIId::Option]["GameObject"].set<bool>("DrawSelf", false);

			s_activeInstance = this;
			installHooksMusicPlayer();

			// stop any playing in-game music
			Object soundMilkyManager = getType("app.sound.SoundMilkyManager")["_Instance"];
			Object sound = soundMilkyManager["_IngameSoundSystem"];
			if (sound != nullptr)
			{
				Object playingBgmInfoList = sound["_PlayingBgmInfoList"];
				std::int32_t playingBgmInfoListLength = playingBgmInfoList.get<std::int32_t>("Length");
				for (std::int32_t i = 0; i < playingBgmInfoListLength; ++i)
				{
					Object playingBgmInfo = playingBgmInfoList[i];
					if (playingBgmInfo != nullptr && playingBgmInfo.get<bool>("IsPlaying"))
					{
						// Which music is playing doesn't matter for stop
						std::optional<MusicSystemMod::BgmTriggerInfo> bgmTriggerInfo = MusicSystemMod::getBgmTriggerInfo(0);
						if (bgmTriggerInfo.has_value())
						{
							Object container = bgmTriggerInfo.value().Container;
							std::uint32_t triggerId = bgmTriggerInfo.value().FadeOutTriggerID;
							Object playerObject = playingBgmInfo["PlayerObject"];

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
						}
					}
				}

				// Process and delete any pending delayed play requests
				Object delayPlayBgmInfoList = sound["_DelayPlayBgmList"];
				std::int32_t delayPlayBgmInfoListLength = delayPlayBgmInfoList.get<std::int32_t>("Length");
				std::uint16_t dummyBgmId = getStaticField<std::uint16_t>("app.cSound_Base.DummyBgmId");
				std::vector<float> pendingPlayBgmTimer(playingBgmInfoListLength, -std::numeric_limits<float>::infinity());
				for (std::int32_t i = 0; i < delayPlayBgmInfoListLength; ++i)
				{
					Object delayPlayBgmInfo = delayPlayBgmInfoList[i];
					if (delayPlayBgmInfo != nullptr)
					{
						std::uint32_t playerID = delayPlayBgmInfo.get<std::uint32_t>("BgmPlayerId");
						float timer = delayPlayBgmInfo.get<float>("Timer");

						// Use the delay request that will fire last
						if (playerID <= playingBgmInfoListLength &&
							timer > pendingPlayBgmTimer[playerID])
						{
							switch ((app::cSound_Base::DelayPlayBgmTypeEnum)delayPlayBgmInfo.get<std::uint8_t>("DelayPlayBgmType"))
							{
							case app::cSound_Base::DelayPlayBgmTypeEnum::ContinuousPlay:
							case app::cSound_Base::DelayPlayBgmTypeEnum::Resume:
								m_pendingDelayBgmRequests[playerID] = delayPlayBgmInfo.get<std::uint32_t>("OriginalId");
								break;
							case app::cSound_Base::DelayPlayBgmTypeEnum::StopTrigger:
								m_pendingDelayBgmRequests[playerID] = dummyBgmId;
								break;
							default:
								break;
							}
						}

						delayPlayBgmInfoList[i].call("reset");
					}
				}
				sound.set<std::uint8_t>("_DelayPlayBgmRequestCount", 0);
			}

			// create SoundLauncherBgmManager if doesn't exist
			if (getType("app.sound.SoundLauncherBgmManager").get("_Instance") == nullptr)
			{
				Object soundLauncherBgmManager = createObject("app.sound.SoundLauncherBgmManager");
				soundLauncherBgmManager.call(".ctor");
				soundLauncherBgmManager["_Manager"] = soundMilkyManager;
				soundLauncherBgmManager["_Container"] = soundMilkyManager["_Container"]; // wrong container but w/e it works
				soundLauncherBgmManager.call("start");

				// create player object
				Object gameObjName = createString(L"SoundLauncherObject");
				Type gameObjType = getType("via.GameObject");
				Object gameObj = Object(gameObjType.call<void *>("create(System.String)", gameObjName));

				// create launcher resource (needed by music player functions)
				Object launcherResource = gameObj.call<Object>("createComponent", getType("app.sound.SoundResourceRegister_Launcher").m_type->get_runtime_type());
				launcherResource["_Manager"] = soundMilkyManager;
				launcherResource["_Container"] = soundMilkyManager["_Container"]; // wrong container but w/e it works
				launcherResource.set<bool>("_IsRegisterResource", true);
				soundMilkyManager["_LauncherResource"] = launcherResource;

				m_ingameMusicPlayer = true;
			}
			else
			{
				m_ingameMusicPlayer = false;
			}

			// call openMusicPlayer()
			launcher.call("openMusicPlayer");

			m_state = State::InMusicPlayer;
		}
		return false;
	}
	case State::InMusicPlayer:
	{
		Object launcher = getSingleton("app.Launcher");
		bool isMusicPlayerEnd = launcher.call<bool>("isMusicPlayerEnd");
		if (isMusicPlayerEnd)
		{
			// call closeMusicPlayer()
			launcher.call("closeMusicPlayer");

			uninstallHooksMusicPlayer();

			// set DrawSelf = true
			launcher["guiBehaviors"][app::Launcher::LauncherGUIId::Option]["GameObject"].set<bool>("DrawSelf", true);

			// call hideLoadingGUIIfShowed()
			launcher.call("hideLoadingGUIIfShowed", false);

			// save playlists
			switch ((CustomPlaylistMenuItem::Option)getValue())
			{
			case CustomPlaylistMenuItem::Option::PreferredMix:
				saveFavoritesList(s_activeInstance->s_PreferredMixPlaylistFileName, s_PreferredMixPlaylist);
				break;
			case CustomPlaylistMenuItem::Option::Playlist:
				if (s_activeInstance->m_customPlaylistFileName.has_value())
				{
					saveFavoritesList(s_activeInstance->m_customPlaylistFileName.value(), m_customPlaylist);
				}
				break;
			default:
				break;
			}

			// destroy SoundLauncherBgmManager if we created it
			if (m_ingameMusicPlayer)
			{
				Object soundMilkyManager = getType("app.sound.SoundMilkyManager")["_Instance"];
				Type componentType = getType("via.Component");

				Type soundLauncherBgmManagerType = getType("app.sound.SoundLauncherBgmManager");
				Object soundLauncherBgmManager = soundLauncherBgmManagerType.get("_Instance");
				if (soundLauncherBgmManager != nullptr)
				{
					soundLauncherBgmManagerType.set("_Instance", Object(nullptr));

					componentType.call("destroy", soundLauncherBgmManager);
				}

				Object launcherResource = soundMilkyManager["_LauncherResource"];
				if (launcherResource != nullptr)
				{
					soundMilkyManager["_LauncherResource"] = nullptr;

					Type gameObjectType = getType("via.GameObject");
					gameObjectType.call("destroy", launcherResource["GameObject"]);
					// this also destroys the component (launcherResource)
				}
			}
			m_ingameMusicPlayer = false;

			// restart any playing in-game music
			Object soundMilkyManager = getType("app.sound.SoundMilkyManager")["_Instance"];
			Object sound = soundMilkyManager["_IngameSoundSystem"];
			if (sound != nullptr)
			{
				Object playingBgmInfoList = sound["_PlayingBgmInfoList"];
				std::int32_t playingBgmInfoListLength = playingBgmInfoList.get<std::int32_t>("Length");

				std::uint16_t dummyBgmId = getStaticField<std::uint16_t>("app.cSound_Base.DummyBgmId");

				for (std::int32_t i = 0; i < playingBgmInfoListLength; ++i)
				{
					Object playingBgmInfo = playingBgmInfoList[i];

					// Pending play request takes priority
					std::uint16_t bgmId = dummyBgmId;
					if (m_pendingDelayBgmRequests.contains(i))
					{
						bgmId = m_pendingDelayBgmRequests[i];
					}
					else if (playingBgmInfo != nullptr &&
							 (playingBgmInfo.get<bool>("IsPlaying")))
					{
						bgmId = playingBgmInfo.get<std::uint16_t>("OriginalId");
					}

					if (bgmId != dummyBgmId)
					{
						sound.call("basePlayBgm", bgmId, 0, i);
					}
				}
			}

			m_pendingDelayBgmRequests.clear();
			s_activeInstance = nullptr;
			m_state = State::Idle;
		}
		return false;
	}
	default:
		return false;
	}
}

/// @brief Install all hooks used for custom playlist editor
void CustomPlaylistMenuItem::installHooksMusicPlayer()
{
	static Object launcherMusicPlayer;

	// Hook method which is called during music player start
	// immediately after setting favorites list and new album flags
	s_hooksMusicPlayer.emplace_back(hook(
		"app.GUILauncherMusicPlayer.setAlbumTrackNum",
		[](int argc, void **argv, auto...)
		{
			launcherMusicPlayer = Object(argv[1]);

			// Count number of new album flags on entry
			s_activeInstance->m_numNewAlbumFlags = countNewAlbumFlags();

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		[](auto...)
		{
			// Set this instance's favorites list
			switch ((CustomPlaylistMenuItem::Option)s_activeInstance->getValue())
			{
			case CustomPlaylistMenuItem::Option::Favorites:
				break;
			case CustomPlaylistMenuItem::Option::Playlist:
				launcherMusicPlayer.set("favoriteMusicList", s_activeInstance->m_customPlaylist);
				break;
			case CustomPlaylistMenuItem::Option::PreferredMix:
				launcherMusicPlayer.set("favoriteMusicList", s_PreferredMixPlaylist);
				break;
			default:
				assert(0);
			}
		}));

	// Hook method which is called to start launcher
	s_hooksMusicPlayer.emplace_back(hook(
		"app.GUILauncherMusicPlayer.start",
		[](int argc, void **argv, auto...)
		{
			launcherMusicPlayer = Object(argv[1]);

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		[](auto...)
		{
			// For Preferred Mix, only show the SF1-3 albums
			if ((CustomPlaylistMenuItem::Option)s_activeInstance->getValue() == CustomPlaylistMenuItem::Option::PreferredMix)
			{
				launcherMusicPlayer["_AlbumTbl"].set<std::int32_t>(0, getStaticField<std::int32_t>("app.GUILauncherMusicPlayer.Album.RR1"));
				launcherMusicPlayer["_AlbumTbl"].set<std::int32_t>(1, getStaticField<std::int32_t>("app.GUILauncherMusicPlayer.Album.RR2"));
				launcherMusicPlayer["_AlbumTbl"].set<std::int32_t>(2, getStaticField<std::int32_t>("app.GUILauncherMusicPlayer.Album.RR3"));
				launcherMusicPlayer["_AlbumList"].set<std::uint32_t>("ItemMax", 3);
				launcherMusicPlayer["_AlbumList"].set<std::uint32_t>("CursorIndex", 1);
			}
		}));

	// Hook method which saves launcher data
	s_hooksMusicPlayer.emplace_back(hook(
		"app.SaveDataManager.requestSaveUserData_Launcher",
		[](auto...)
		{
			// If we are not editing the favorites list, do a save
			if (s_activeInstance == nullptr)
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			// If we are editing the actual favorites list, do a save
			if ((CustomPlaylistMenuItem::Option)s_activeInstance->getValue() == CustomPlaylistMenuItem::Option::Favorites)
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			// If number of new album flags changed, do a save
			// Otherwise we don't need to save
			if (s_activeInstance->m_numNewAlbumFlags != countNewAlbumFlags())
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			// Otherwise we don't need to save
			return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
		},
		nullptr));
}

/// @brief Uninstall all hooks used for custom playlist editor
void CustomPlaylistMenuItem::uninstallHooksMusicPlayer()
{
	for (HookRef &hook : s_hooksMusicPlayer)
	{
		hook.unhook();
	}
	s_hooksMusicPlayer.clear();
}

/// @brief Count number of new album flags which are 1
/// @return Number of flags which are 1
std::size_t CustomPlaylistMenuItem::countNewAlbumFlags()
{
	Object musicAlbumNewFlags = getSingleton("app.Launcher")["_saveData"]["musicAlbumNewFlags"];
	std::size_t length = musicAlbumNewFlags.get<std::int32_t>("Length");
	std::size_t numNewAlbumFlags = 0;
	for (std::size_t i = 0; i < length; ++i)
	{
		numNewAlbumFlags += musicAlbumNewFlags.get<bool>(i);
	}
	return numNewAlbumFlags;
}
