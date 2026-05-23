#pragma once

#include <cstdint>
#include <vector>

#include "Guid.hpp"

struct MenuItem
{
public:
	const Guid NameGuid;
	const Guid DescriptionGuid;
	const std::vector<Guid> *SelectionOptions;
	uint8_t *const SettingPtr;
};
