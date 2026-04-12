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
		&this->data1,
		&this->data2,
		&this->data3,
		&this->data4_0, &this->data4_1,
		&this->data4_2, &this->data4_3, &this->data4_4, &this->data4_5, &this->data4_6, &this->data4_7);
}

/// @brief Convert GUID to string.
/// @return Formatted GUID string.
std::wstring Guid::toString() const
{
	return std::format(
		GUID_FORMAT_STRING,
		this->data1,
		this->data2,
		this->data3,
		this->data4_0, this->data4_1,
		this->data4_2, this->data4_3, this->data4_4, this->data4_5, this->data4_6, this->data4_7);
}

/// @brief Compare two GUIDs.
/// @param other GUID to compare with.
/// @return 0 if GUIDs are equal, -1 if this GUID is smaller, 1 if it is greater.
std::strong_ordering Guid::compare(Guid const &other) const
{
	// Zero one of the fields extend to 64-bit
	std::int64_t result = (std::int64_t)(std::uint64_t)this->data1 - other.data1;
	if (result == 0)
	{
		result = (std::int64_t)(std::uint64_t)this->data2 - other.data2;
		if (result == 0)
		{
			result = (std::int64_t)(std::uint64_t)this->data3 - other.data3;
			if (result == 0)
			{
				result = (std::int64_t)(std::uint64_t)this->data4L_0 - other.data4L_0;
				if (result == 0)
				{
					result = (std::int64_t)(std::uint64_t)this->data4L_1 - other.data4L_1;
				}
			}
		}
	}
	return result == 0 ? std::strong_ordering::equal : result < 0 ? std::strong_ordering::less
																  : std::strong_ordering::greater;
}
