#include <Enums_Internal.hpp>

#include <filesystem>
#include <fstream>
#include <utility>

#include "CustomPlaylistMenuItem.hpp"
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

Object CustomPlaylistMenuItem::getPreferMixPlaylist()
{
	return s_PreferMixPlaylist;
}

void CustomPlaylistMenuItem::setPreferMixPlaylist(Object playlist)
{
	if (s_PreferMixPlaylist)
	{
		s_PreferMixPlaylist.m_object->release();
	}

	s_PreferMixPlaylist = playlist;

	if (s_PreferMixPlaylist)
	{
		s_PreferMixPlaylist.m_object->add_ref();
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
			favoritesList.call("Add(app.GUILauncherMusicPlayer.MusicInfo)", {musicInfo});
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

	return true;
}

const std::filesystem::path &CustomPlaylistMenuItem::getPreferMixPlaylistFileName()
{
	return s_PreferMixPlaylistFileName;
}
void CustomPlaylistMenuItem::setPreferMixPlaylistFileName(const std::filesystem::path &fileName)
{
	s_PreferMixPlaylistFileName = fileName;
}

CustomPlaylistMenuItem::CustomPlaylistMenuItem(const std::filesystem::path &playlistFileName, Guid nameGuid, Guid descriptionGuid, const std::vector<MenuItem::Option> *options, std::int32_t *valuePtr, std::int32_t defaultValue)
	: MenuItem::MenuItem(nameGuid, descriptionGuid, options, valuePtr, defaultValue), m_customPlaylistFileName(playlistFileName)
{
	// Load original/arranged playlist if this is the first instance
	if (!s_PreferMixPlaylist)
	{
		setPreferMixPlaylist(loadFavoritesList(s_PreferMixPlaylistFileName));
	}

	// Load custom playlist
	setCustomPlaylist(loadFavoritesList(m_customPlaylistFileName));
}

CustomPlaylistMenuItem::~CustomPlaylistMenuItem()
{
	setCustomPlaylist((void *)nullptr);
}

bool CustomPlaylistMenuItem::onEnter()
{
	if (m_state != State::Idle)
	{
		return false;
	}

	// Check if the current value is a playlist that can be edited
	switch (getValue())
	{
	case std::to_underlying(CustomPlaylistMenuItem::Option::PreferMix):
	case std::to_underlying(CustomPlaylistMenuItem::Option::Playlist):
	case std::to_underlying(CustomPlaylistMenuItem::Option::Favorites):
		break;
	default:
		return false;
	}

	Object launcher = getSingleton("app.Launcher");
	launcher.call(
		"showLoadingGUI(app.LauncherFadeUserData.FadeUsage, System.Boolean, System.Boolean)",
		{
			(void *)(intptr_t)app::LauncherFadeUserData::FadeUsage::GalleryToMusicPlayer2Way,
			(void *)(intptr_t)false,
			(void *)(intptr_t)false,
		});

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
		bool isFadeInAnimEnd = launcher["loadingGuiBehavior"].call<bool>("isFadeInAnimEnd()");
		if (isFadeInAnimEnd)
		{
			// set DrawSelf = false
			launcher["guiBehaviors"][app::Launcher::LauncherGUIId::Option]["GameObject"].set<bool>("DrawSelf", false);

			s_activeInstance = this;
			installHooks();

			// call openMusicPlayer()
			launcher.call("openMusicPlayer()");

			m_state = State::InMusicPlayer;
		}
		return false;
	}
	case State::InMusicPlayer:
	{
		Object launcher = getSingleton("app.Launcher");
		bool isMusicPlayerEnd = launcher.call<bool>("isMusicPlayerEnd()");
		if (isMusicPlayerEnd)
		{
			// call closeMusicPlayer()
			launcher.call("closeMusicPlayer()");

			uninstallHooks();

			// set DrawSelf = true
			launcher["guiBehaviors"][app::Launcher::LauncherGUIId::Option]["GameObject"].set<bool>("DrawSelf", true);

			// call hideLoadingGUIIfShowed()
			launcher.call(
				"hideLoadingGUIIfShowed(System.Boolean)",
				{
					(void *)(intptr_t)false,
				});

			// save playlists
			saveFavoritesList(s_activeInstance->m_customPlaylistFileName, m_customPlaylist);
			saveFavoritesList(s_PreferMixPlaylistFileName, s_PreferMixPlaylist);

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
void CustomPlaylistMenuItem::installHooks()
{
	// Hook method which starts the music player
	static Object launcherMusicPlayer;
	s_hooks.emplace_back(hook(
		"app.GUILauncherMusicPlayer.start()",
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
			if (s_activeInstance->getValue() == std::to_underlying(CustomPlaylistMenuItem::Option::Playlist))
			{
				launcherMusicPlayer.set("favoriteMusicList", s_activeInstance->m_customPlaylist);
			}
			else if (s_activeInstance->getValue() == std::to_underlying(CustomPlaylistMenuItem::Option::PreferMix))
			{
				launcherMusicPlayer.set("favoriteMusicList", s_PreferMixPlaylist);
			}
		}));

	// Hook method which saves launcher data to avoid saving
	s_hooks.emplace_back(hook(
		"app.SaveDataManager.requestSaveUserData_Launcher",
		[](auto...)
		{
			// If we are editing the actual favorites list, do a save
			if (s_activeInstance->getValue() == std::to_underlying(CustomPlaylistMenuItem::Option::Favorites))
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
void CustomPlaylistMenuItem::uninstallHooks()
{
	for (HookRef &hook : s_hooks)
	{
		hook.unhook();
	}
	s_hooks.clear();
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
