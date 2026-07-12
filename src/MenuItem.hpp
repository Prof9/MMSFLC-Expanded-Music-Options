#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "Guid.hpp"

struct MenuItem
{
	struct Option
	{
		const Guid m_nameGuid;
		const std::int32_t m_value;
		const std::optional<Guid> m_descriptionGuid;
	};

public:
	const Guid m_nameGuid;
	const Guid m_descriptionGuid;
	const std::vector<Option> *m_options;

	std::int32_t *const m_value;

	std::int32_t const m_defaultValue = 0;

	MenuItem(Guid nameGuid, Guid descriptionGuid, const std::vector<Option> *options, std::int32_t *valuePtr, std::int32_t defaultValue = 0)
		: m_nameGuid(nameGuid), m_descriptionGuid(descriptionGuid), m_options(options), m_value(valuePtr), m_defaultValue(defaultValue) {}

	virtual ~MenuItem() = default;

	std::int32_t getValue() const;
	void setValue(std::int32_t value) const;
	std::int32_t getCursor() const;

	virtual bool onEnter();
	virtual bool onUpdate();
};
