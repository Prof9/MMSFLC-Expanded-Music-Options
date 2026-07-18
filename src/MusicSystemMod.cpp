#include <Enums_Internal.hpp>

#include <memory>

#include "MessageManager.hpp"
#include "MusicSystemMod.hpp"
#include "REFrameworkHelper.hpp"

using namespace REFrameworkHelper;

/// @brief Early initialization of music system which can be done before game has fully launched
void MusicSystemMod::init()
{
	if (s_initialized)
	{
		return;
	}
	s_initialized = true;

	loadSettings();
}

/// @brief Install music system mod after game has launched
void MusicSystemMod::install()
{
	init();

	// Load strings
	MessageManager messageManager(getNativeSingleton("via.gui.MessageManager"));
	messageManager.createAndLoadMessages(MusicSystemMod::STRINGS, via::Language::English);

	buildMenu();

	installHooks();
}

/// @brief Uninstall music system mod
void MusicSystemMod::uninstall()
{
	uninstallHooks();
}

/// @brief Play BGM
/// @param srcObj Player object
/// @param musicId BGM ID
/// @param isArranged Whether to play arranged or original mix
/// @return True if BGM started, false if BGM could not be started
bool MusicSystemMod::playBgm(Object srcObj, std::uint16_t bgmId, bool isArranged)
{
	return false;
}
