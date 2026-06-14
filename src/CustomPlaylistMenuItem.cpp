#include <Enums_Internal.hpp>

#include <utility>

#include "CustomPlaylistMenuItem.hpp"
#include "REFrameworkHelper.hpp"

using namespace REFrameworkHelper;

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
