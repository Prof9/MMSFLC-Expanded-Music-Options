#pragma once

#include "MenuItem.hpp"
#include "REFrameworkHelper.hpp"

struct CustomPlaylistMenuItem : MenuItem
{
	enum struct State
	{
		Idle,
		OpeningMusicPlayer,
		InMusicPlayer,
	};

public:
	CustomPlaylistMenuItem(Guid nameGuid, Guid descriptionGuid, const std::vector<Guid> *valueNames, std::int32_t *valuePtr, std::int32_t defaultValue = 0);
	virtual ~CustomPlaylistMenuItem();

	bool onEnter();
	bool onUpdate();

	REFrameworkHelper::Object m_favoritesList;

protected:
	static void installHooks();
	static void uninstallHooks();

	static std::size_t countNewAlbumFlags();

private:
	// Instance state
	State m_state = State::Idle;
	std::size_t m_numNewAlbumFlags;

	// Static state
	static inline CustomPlaylistMenuItem *s_activeInstance;

	// Hooks
	static inline std::vector<REFrameworkHelper::HookRef> s_hooks;
};
