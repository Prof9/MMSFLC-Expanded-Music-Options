#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include <reframework/API.hpp>

#include "MenuExtension.hpp"
#include "REFrameworkHelper.hpp"

class AudioMenuExtension : public MenuExtension<AudioMenuExtension>
{
	friend class MenuExtension;

public:
	/// @brief Create new menu extension.
	/// @param newMenuItems New menu items which are part of this extension.
	AudioMenuExtension(std::span<MenuItem const> newMenuItems)
		: MenuExtension(newMenuItems) {}

protected:
	static void installHooks();
	static void uninstallHooks();

private:
	// Constants
	static const inline size_t ITEMS_PER_PAGE = 6;

	// Static state
	static inline REFrameworkHelper::Object s_menuTblOption;
	static inline int s_newSoundOptionsIdx = -1;
	static inline bool s_doResizeFluentScrollList = false;

	// Hooks
	static inline std::vector<REFrameworkHelper::HookRef> s_hooks;
};
