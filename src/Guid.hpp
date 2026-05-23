#pragma once

#include <array>
#include <compare>
#include <cstdint>
#include <string>

#include "REFrameworkHelper.hpp"

#pragma pack(push, 1)
struct Guid
{
	friend struct REFrameworkHelper::Object;

public:
	std::uint32_t m_data1;
	std::uint16_t m_data2;
	std::uint16_t m_data3;
	std::uint8_t m_data4_0;
	std::uint8_t m_data4_1;
	std::uint8_t m_data4_2;
	std::uint8_t m_data4_3;
	std::uint8_t m_data4_4;
	std::uint8_t m_data4_5;
	std::uint8_t m_data4_6;
	std::uint8_t m_data4_7;

	Guid() = default;
	Guid(std::uint32_t data1, std::uint16_t data2, std::uint16_t data3, std::uint16_t data4_01, std::uint64_t data4_234567)
		: m_data1(data1), m_data2(data2), m_data3(data3), m_data4_0(data4_01 >> 8), m_data4_1(data4_01),
		  m_data4_2(data4_234567 >> 40), m_data4_3(data4_234567 >> 32), m_data4_4(data4_234567 >> 24),
		  m_data4_5(data4_234567 >> 16), m_data4_6(data4_234567 >> 8), m_data4_7(data4_234567) {};
	Guid(std::wstring const &str);
	Guid(void *ptr)
		: m_data1(*(std::uint32_t *)((intptr_t)ptr + 0x0)),
		  m_data2(*(std::uint16_t *)((intptr_t)ptr + 0x4)),
		  m_data3(*(std::uint16_t *)((intptr_t)ptr + 0x6)),
		  m_data4_0(*(std::uint8_t *)((intptr_t)ptr + 0x8)),
		  m_data4_1(*(std::uint8_t *)((intptr_t)ptr + 0x9)),
		  m_data4_2(*(std::uint8_t *)((intptr_t)ptr + 0xA)),
		  m_data4_3(*(std::uint8_t *)((intptr_t)ptr + 0xB)),
		  m_data4_4(*(std::uint8_t *)((intptr_t)ptr + 0xC)),
		  m_data4_5(*(std::uint8_t *)((intptr_t)ptr + 0xD)),
		  m_data4_6(*(std::uint8_t *)((intptr_t)ptr + 0xE)),
		  m_data4_7(*(std::uint8_t *)((intptr_t)ptr + 0xF)) {}

	std::wstring toString() const;

	friend bool operator==(Guid const &a, Guid const &b) = default;
	friend std::strong_ordering operator<=>(Guid const &a, Guid const &b) = default;

protected:
	Guid(REFrameworkHelper::ValueTypeArray array)
		: m_data1(array.at(0) | array.at(1) << 8 | array.at(2) << 16 | array.at(3) << 24),
		  m_data2(array.at(4) | array.at(5) << 8),
		  m_data3(array.at(6) | array.at(7) << 8),
		  m_data4_0(array.at(8)),
		  m_data4_1(array.at(9)),
		  m_data4_2(array.at(10)),
		  m_data4_3(array.at(11)),
		  m_data4_4(array.at(12)),
		  m_data4_5(array.at(13)),
		  m_data4_6(array.at(14)),
		  m_data4_7(array.at(15)) {}
	operator REFrameworkHelper::ValueTypeArray() const
	{
		REFrameworkHelper::ValueTypeArray array = {0};
		array[0] = m_data1;
		array[1] = m_data1 >> 8;
		array[2] = m_data1 >> 16;
		array[3] = m_data1 >> 24;
		array[4] = m_data2;
		array[5] = m_data2 >> 8;
		array[6] = m_data3;
		array[7] = m_data3 >> 8;
		array[8] = m_data4_0;
		array[9] = m_data4_1;
		array[10] = m_data4_2;
		array[11] = m_data4_3;
		array[12] = m_data4_4;
		array[13] = m_data4_5;
		array[14] = m_data4_6;
		array[15] = m_data4_7;
		return array;
	}
};
#pragma pack(pop)

Guid operator""_guid(const wchar_t *s, size_t);
