#pragma once

#include <cstdint>
#include <uchar.h>

#include "Guid.hpp"

struct MessageReference;

#pragma pack(push, 1)
struct MessageFileEntry
{
	Guid guid;
	uint32_t soundID;
	uint32_t hash;
	char16_t const *name;
	void const *attributes;
	char16_t const *strings[];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MessageFile
{
	uint32_t entryCount;
	uint32_t attributeCount;
	uint32_t languageCount;
	uint8_t field_0x0C[4];
	void *data;
	void *unknownData;
	uint32_t *languages;
	void *attributes;
	void *attributeNames;
	MessageFileEntry *entries[];

	MessageReference createMessageReference(Guid const &guid);
};
#pragma pack(pop)