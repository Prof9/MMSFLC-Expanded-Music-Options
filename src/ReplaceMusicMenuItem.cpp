#include "ReplaceMusicMenuItem.hpp"
#include "MusicSystemMod.hpp"

void ReplaceMusicMenuItem::setValue(std::int32_t value) const
{
	MenuItem::setValue(value);

	MusicSystemMod::reloadBgmForMenuItem(this, true);
}
