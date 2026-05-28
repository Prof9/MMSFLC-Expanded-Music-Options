#pragma once

#include <cstdint>
#include <vector>

#include "Guid.hpp"

struct MenuItem
{
public:
	const Guid m_nameGuid;
	const Guid m_descriptionGuid;
	const std::vector<Guid> *m_valueNames;

	std::int32_t *const m_value;

	std::int32_t const m_defaultValue = 0;

	std::int32_t getValue() const;
	void setValue(std::int32_t) const;
};
