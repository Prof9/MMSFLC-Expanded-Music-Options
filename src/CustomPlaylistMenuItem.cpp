#include <Enums_Internal.hpp>

#include <utility>

#include "CustomPlaylistMenuItem.hpp"
#include "REFrameworkHelper.hpp"

using namespace REFrameworkHelper;

CustomPlaylistMenuItem::CustomPlaylistMenuItem(Guid nameGuid, Guid descriptionGuid, const std::vector<Guid> *valueNames, std::int32_t *valuePtr, std::int32_t defaultValue)
	: MenuItem::MenuItem(nameGuid, descriptionGuid, valueNames, valuePtr, defaultValue)
{
	// Create new favorites list
	m_favoritesList = createObject("System.Collections.Generic.List`1<app.GUILauncherMusicPlayer.MusicInfo>");
	m_favoritesList.call<void>(".ctor()");
	m_favoritesList.m_object->add_ref();
}

CustomPlaylistMenuItem::~CustomPlaylistMenuItem()
{
	m_favoritesList.m_object->release();
}

bool CustomPlaylistMenuItem::onEnter()
{
	if (m_state != State::Idle)
	{
		return false;
	}

	Object launcher = getSingleton("app.Launcher");
	launcher.call<void>(
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
		Object loading = launcher.get<Object>("loadingGuiBehavior");
		bool isFadeInAnimEnd = loading.call<bool>("isFadeInAnimEnd()");
		if (isFadeInAnimEnd)
		{
			// set DrawSelf = false
			Object guiBehaviors = launcher.get<Object>("guiBehaviors");
			Object guiLauncherOption = guiBehaviors.get<Object>(std::to_underlying(app::Launcher::LauncherGUIId::Option));
			Object guiLauncherOptionObject = guiLauncherOption.get<Object>("GameObject");
			guiLauncherOptionObject.set<bool>("DrawSelf", false);

			s_activeInstance = this;
			installHooks();

			// call openMusicPlayer()
			launcher.call<void>("openMusicPlayer()");

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
			launcher.call<void>("closeMusicPlayer()");

			uninstallHooks();
			s_activeInstance = nullptr;

			// set DrawSelf = true
			Object guiBehaviors = launcher.get<Object>("guiBehaviors");
			Object guiLauncherOption = guiBehaviors.get<Object>(std::to_underlying(app::Launcher::LauncherGUIId::Option));
			Object guiLauncherOptionObject = guiLauncherOption.get<Object>("GameObject");
			guiLauncherOptionObject.set<bool>("DrawSelf", true);

			// call hideLoadingGUIIfShowed()
			launcher.call<void>(
				"hideLoadingGUIIfShowed(System.Boolean)",
				{
					(void *)(intptr_t)false,
				});

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
			launcherMusicPlayer.set<Object>("favoriteMusicList", s_activeInstance->m_favoritesList);

			launcherMusicPlayer.m_object = nullptr;
		}));

	// Hook method which saves launcher data to avoid saving
	s_hooks.emplace_back(hook(
		"app.SaveDataManager.requestSaveUserData_Launcher",
		[](auto...)
		{
			// If number of new albm flags changed, do a save
			// Otherwise we don't need to save
			if (s_activeInstance->m_numNewAlbumFlags != countNewAlbumFlags())
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}
			else
			{
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}
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
	Object musicAlbumNewFlags = getSingleton("app.Launcher").get<Object>("_saveData").get<Object>("musicAlbumNewFlags");
	std::size_t length = musicAlbumNewFlags.get<std::int32_t>("Length");
	std::size_t numNewAlbumFlags = 0;
	for (std::size_t i = 0; i < length; ++i)
	{
		numNewAlbumFlags += musicAlbumNewFlags.get<bool>(i);
	}
	return numNewAlbumFlags;
}
