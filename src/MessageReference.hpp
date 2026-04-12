#pragma once

#include <Enums_Internal.hpp>

#include <cstdint>
#include <uchar.h>

#include "MessageFile.hpp"

#pragma pack(push, 1)
struct MessageReference
{
	friend struct MessageFile;

private:
	const MessageFile *messageFile;
	const uint64_t messageIndex;

	MessageReference()
		: messageFile(nullptr), messageIndex(0) {};
	MessageReference(MessageFile *messageFile, std::uint64_t messageIndex)
		: messageFile(messageFile), messageIndex(messageIndex) {};

	char16_t const **getStringPointer(via::Language language) const;

public:
	char16_t const *getString(via::Language language) const;
	bool setString(via::Language language, char16_t const *string) const;
	bool setStringAllLanguages(char16_t const *string) const;

	bool isValid() const
	{
		return this->messageFile != nullptr && this->messageIndex < this->messageFile->entryCount;
	}
};
#pragma pack(pop)
