#pragma once

#include "MenuItem.hpp"

struct ReplaceMusicMenuItem : MenuItem
{
public:
	using MenuItem::MenuItem;

	virtual void setValue(std::int32_t value) const;
};
