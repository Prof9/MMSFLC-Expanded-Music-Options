#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
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

	MenuItem(Guid nameGuid, Guid descriptionGuid, const std::vector<Guid> *valueNames, std::int32_t *valuePtr, std::int32_t defaultValue = 0)
		: m_nameGuid(nameGuid), m_descriptionGuid(descriptionGuid), m_valueNames(valueNames), m_value(valuePtr), m_defaultValue(defaultValue) {}

	virtual ~MenuItem() = default;

	std::int32_t getValue() const;
	void setValue(std::int32_t) const;

	virtual bool onEnter();
	virtual bool onUpdate();
};
