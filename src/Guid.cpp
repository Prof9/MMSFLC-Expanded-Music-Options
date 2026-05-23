#include <cinttypes>
#include <compare>
#include <cstdio>
#include <format>
#include <string>

#include "Guid.hpp"

#define GUID_FORMAT_STRING L"%08" PRIx32 "-%04" PRIx16 "-%04" PRIx16 "-%02" PRIx8 "%02" PRIx8 "-%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8

/// @brief Construct GUID from string.
/// @param string String formatted like: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" where x is a hex character.
Guid::Guid(std::wstring const &string)
{
	std::swscanf(
		string.c_str(),
		GUID_FORMAT_STRING,
		&this->m_data1,
		&this->m_data2,
		&this->m_data3,
		&this->m_data4_0, &this->m_data4_1,
		&this->m_data4_2, &this->m_data4_3, &this->m_data4_4, &this->m_data4_5, &this->m_data4_6, &this->m_data4_7);
}

/// @brief Convert GUID to string.
/// @return Formatted GUID string.
std::wstring Guid::toString() const
{
	return std::format(
		GUID_FORMAT_STRING,
		this->m_data1,
		this->m_data2,
		this->m_data3,
		this->m_data4_0, this->m_data4_1,
		this->m_data4_2, this->m_data4_3, this->m_data4_4, this->m_data4_5, this->m_data4_6, this->m_data4_7);
}

Guid operator""_guid(const wchar_t *s, size_t)
{
	return Guid(s);
}
