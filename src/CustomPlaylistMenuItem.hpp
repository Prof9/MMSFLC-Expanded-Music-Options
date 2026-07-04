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
	CustomPlaylistMenuItem(const std::filesystem::path &playlistFileName, Guid nameGuid, Guid descriptionGuid, const std::vector<Guid> *valueNames, std::int32_t *valuePtr, std::int32_t defaultValue = 0);
	virtual ~CustomPlaylistMenuItem();

	bool onEnter();
	bool onUpdate();

	REFrameworkHelper::Object getCustomPlaylist() const;
	void setCustomPlaylist(REFrameworkHelper::Object playlist);

	static REFrameworkHelper::Object getOriginalArrangedPlaylist();
	static void setOriginalArrangedPlaylist(REFrameworkHelper::Object playlist);

	static REFrameworkHelper::Object loadFavoritesList(const std::filesystem::path &fileName);
	static bool saveFavoritesList(const std::filesystem::path &fileName, REFrameworkHelper::Object favoritesList);

	static const std::filesystem::path &getOriginalArrangedPlaylistFileName();
	static void setOriginalArrangedPlaylistFileName(const std::filesystem::path &fileName);

protected:
	static void installHooks();
	static void uninstallHooks();

	static std::size_t countNewAlbumFlags();

private:
	// Instance state
	State m_state = State::Idle;
	std::size_t m_numNewAlbumFlags;

	std::filesystem::path m_customPlaylistFileName;
	REFrameworkHelper::Object m_customPlaylist;

	// Static state
	static inline CustomPlaylistMenuItem *s_activeInstance;

	static inline std::filesystem::path s_originalArrangedPlaylistFileName;
	static inline REFrameworkHelper::Object s_originalArrangedPlaylist;

	// Hooks
	static inline std::vector<REFrameworkHelper::HookRef> s_hooks;
};
