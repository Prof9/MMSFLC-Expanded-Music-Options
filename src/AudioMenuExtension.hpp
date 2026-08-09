#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <utility>

#include <reframework/API.hpp>

#include "CustomPlaylistMenuItem.hpp"
#include "MenuExtension.hpp"
#include "REFrameworkHelper.hpp"

class AudioMenuExtension : public MenuExtension<AudioMenuExtension>
{
	friend class MenuExtension;
	friend class MenuExtension<AudioMenuExtension>;

public:
	using MenuExtension::MenuExtension;

	static void updateList(REFrameworkHelper::Object);
	static void updateGuidMessage(REFrameworkHelper::Object);

protected:
	static void installHooks();
	static void uninstallHooks();

	static bool isBgmSelectionFavoritesSelected();

private:
	// Constants
	static const inline size_t ITEMS_PER_PAGE = 6;

	// Dummy CustomPlaylistMenuItem
	// This is used to enter Music Player on base game BGM Selection
	static inline const std::vector<MenuItem::Option> s_dummyCustomPlaylistMenuItemOptions = {
		{
			L"00000000-0000-0000-0000-000000000000"_guid,
			std::to_underlying(CustomPlaylistMenuItem::Option::Favorites),
			L"00000000-0000-0000-0000-000000000000"_guid,
		},
	};
	static inline std::int32_t s_dummyCustomPlaylistMenuItemValue = std::to_underlying(CustomPlaylistMenuItem::Option::Favorites);
	static inline std::shared_ptr<CustomPlaylistMenuItem> s_dummyCustomPlaylistMenuItem; // must be initialized at runtime

	// Hooks
	static inline std::vector<REFrameworkHelper::HookRef> s_hooks;
};
