#include <Enums_Internal.hpp>

#include <compare>
#include <cstdint>
#include <map>
#include <format>
#include <string>

#include <Windows.h>

#include "Guid.hpp"
#include "MessageManager.hpp"
#include "MessageReference.hpp"
#include "MessageUtility.hpp"

#pragma pack(push, 1)
struct Node
{
	Node *childLesser;
	Node *parent;
	Node *childGreater;
	uint8_t field_0x18;
	uint8_t isInvalid;
	uint8_t field_0x1A[6];

	Guid guid;
	MessageReference messageReference;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Tree
{
	Node *minChild;
	Node *root;
	Node *maxChild;
	uint8_t field_0x18;
	uint8_t isInvalid;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MessageManager::NativeObject
{
	uint8_t field_0x00[0x40];
	Tree *messageTree;
};
#pragma pack(pop)

/// @brief Gets a reference to a loaded message with the given GUID.
/// @param guid GUID of message.
/// @return Message reference, if found, otherwise `nullptr`.
MessageReference *MessageManager::getMessageReference(Guid const &guid) const
{
	Node *node = this->nativeObject->messageTree->root;
	while (true)
	{
		if (node == nullptr || node->isInvalid)
		{
			std::wstring guidString = guid.toString();
			OutputDebugString(std::vformat(
								  L"Message with GUID %s not found",
								  std::make_format_args<std::wformat_context>(
									  guidString))
								  .c_str());
			return nullptr;
		}

		std::strong_ordering ordering = guid.compare(node->guid);
		if (ordering == std::strong_ordering::equal)
		{
			break;
		}
		else if (ordering == std::strong_ordering::greater)
		{
			node = node->childGreater;
		}
		else
		{
			node = node->childLesser;
		}
	}

	return &node->messageReference;
}

/// @brief Replace message by name in multiple languages.
/// @param name Message name.
/// @param languageStrings Mapping of language to string.
/// @param fillRemainingWithLanguage If set to one of the languages provided in `languageStrings`, then all language strings which are not in `languageStrings` will be copied from that language.
/// @return `true` if all replacements succeeded, `false` if any failed.
bool MessageManager::replaceMessageByName(std::string name, std::map<via::Language, char16_t const *> languageStrings, via::Language fillRemainingWithLanguage) const
{
	Guid *guid = MessageUtility::getMessageGuidByName(name);
	if (guid == nullptr)
	{
		return false;
	}

	MessageReference *messageReference = this->getMessageReference(*guid);
	if (messageReference == nullptr)
	{
		return false;
	}

	bool error = false;
	if (languageStrings.contains(fillRemainingWithLanguage))
	{
		if (!messageReference->setStringAllLanguages(languageStrings[fillRemainingWithLanguage]))
		{
			error = true;
		}
	}
	for (auto it = languageStrings.begin(); it != languageStrings.end(); ++it)
	{
		if (!messageReference->setString(it->first, it->second))
		{
			error = true;
		}
	}
	return !error;
}
