#include <Enums_Internal.hpp>

#include <algorithm>
#include <cstdint>

#include "MenuItem.hpp"

std::int32_t MenuItem::getValue() const
{
	std::int32_t value = *m_value;
	if (m_options->end() == std::find_if(m_options->begin(), m_options->end(), [value](MenuItem::Option option)
										 { return option.m_value == value; }))
	{
		value = m_defaultValue;
	};
	return value;
}

void MenuItem::setValue(std::int32_t value) const
{
	if (m_options->end() == std::find_if(m_options->begin(), m_options->end(), [value](MenuItem::Option option)
										 { return option.m_value == value; }))
	{
		value = m_defaultValue;
	};
	*m_value = value;
}

std::int32_t MenuItem::getCursor() const
{
	std::int32_t value = getValue();
	for (std::int32_t i = 0; i < m_options->size(); i++)
	{
		if ((*m_options)[i].m_value == value)
		{
			return i;
		}
	}
	return 0;
}

bool MenuItem::onEnter()
{
	return false;
}

bool MenuItem::onUpdate()
{
	return true;
}