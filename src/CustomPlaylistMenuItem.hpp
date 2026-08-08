#pragma once

#include <filesystem>

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

	enum struct Option : std::int32_t
	{
		NoChange,
		MusicOff,
		AlwaysOriginal,
		AlwaysArranged,
		PreferredMix,
		Playlist,
		Favorites,
		Field,
		FieldOriginal,
		FieldArranged,
		Battle,
	};

public:
	CustomPlaylistMenuItem(const std::filesystem::path &playlistFileName, Guid nameGuid, Guid descriptionGuid, const std::vector<MenuItem::Option> *options, std::int32_t *valuePtr, std::int32_t defaultValue = 0);
	virtual ~CustomPlaylistMenuItem();

	virtual void setValue(std::int32_t value) const;

	bool canEnter();
	bool onEnter();
	bool onUpdate();

	REFrameworkHelper::Object getCustomPlaylist() const;
	void setCustomPlaylist(REFrameworkHelper::Object playlist);

	static REFrameworkHelper::Object getPreferredMixPlaylist();
	static void setPreferredMixPlaylist(REFrameworkHelper::Object playlist);

	static REFrameworkHelper::Object loadFavoritesList(const std::filesystem::path &fileName);
	static bool saveFavoritesList(const std::filesystem::path &fileName, REFrameworkHelper::Object favoritesList);

	static const std::filesystem::path &getPreferredMixPlaylistFileName();
	static void setPreferredMixPlaylistFileName(const std::filesystem::path &fileName);

protected:
	static void installHooksMusicPlayer();
	static void uninstallHooksMusicPlayer();

	static std::size_t countNewAlbumFlags();

private:
	// Instance state
	State m_state = State::Idle;
	bool m_ingameMusicPlayer = false;
	std::size_t m_numNewAlbumFlags;
	std::unordered_map<std::size_t, std::uint16_t> m_pendingDelayBgmRequests;

	std::filesystem::path m_customPlaylistFileName;
	REFrameworkHelper::Object m_customPlaylist;

	// Static state
	static inline CustomPlaylistMenuItem *s_activeInstance;

	static inline std::filesystem::path s_PreferredMixPlaylistFileName;
	static inline REFrameworkHelper::Object s_PreferredMixPlaylist;

	// Hooks
	static inline std::vector<REFrameworkHelper::HookRef> s_hooksMusicPlayer;
};
