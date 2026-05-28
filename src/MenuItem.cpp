#include <cstdint>

#include "MenuItem.hpp"

std::int32_t MenuItem::getValue() const
{
	std::int32_t value = *m_value;
	if (value < 0 || value >= m_valueNames->size())
	{
		value = m_defaultValue;
	}
	return value;
}

void MenuItem::setValue(std::int32_t value) const
{
	if (value < 0 || value >= m_valueNames->size())
	{
		value = m_defaultValue;
	}
	*m_value = value;
}