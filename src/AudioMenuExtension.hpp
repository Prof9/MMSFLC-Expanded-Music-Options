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
	friend class MenuExtension<AudioMenuExtension>;

public:
	using MenuExtension::MenuExtension;

	static void updateList(REFrameworkHelper::Object);
	static void updateGuidMessage(REFrameworkHelper::Object);

protected:
	static void installHooks();
	static void uninstallHooks();

private:
	// Constants
	static const inline size_t ITEMS_PER_PAGE = 6;

	// Static state
	static inline int s_newSoundOptionsIdx = -1; // Also indicates if menu has been resized

	// Hooks
	static inline std::vector<REFrameworkHelper::HookRef> s_hooks;
};
