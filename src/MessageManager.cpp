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
struct MessageManager::Node
{
	MessageManager::Node *childLesser;
	MessageManager::Node *parent;
	MessageManager::Node *childGreater;
	uint8_t field_0x18;
	uint8_t isInvalid;
	uint8_t field_0x1A[6];

	Guid guid;
	MessageReference messageReference;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MessageManager::Tree
{
	MessageManager::Node *minChild;
	MessageManager::Node *root;
	MessageManager::Node *maxChild;
	uint8_t field_0x18;
	uint8_t isInvalid;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MessageManager::NativeObject
{
	uint8_t field_0x00[0x40];
	MessageManager::Tree *messageTree;
};
#pragma pack(pop)

/// @brief Gets closest nodes for given GUID
/// @param guid GUID
/// @return Returns found node (can be nullptr) and parent of found node
std::pair<MessageManager::Node *, MessageManager::Node *> MessageManager::findNode(Guid const &guid) const
{
	MessageManager::Node *node = this->nativeObject->messageTree->root;
	MessageManager::Node *prevNode = node;
	while (true)
	{
		if (node == nullptr || node->isInvalid)
		{
			std::wstring guidString = guid.toString();
			node = nullptr;
			break;
		}

		std::strong_ordering ordering = guid <=> node->guid;
		if (ordering == std::strong_ordering::equal)
		{
			break;
		}
		else if (ordering == std::strong_ordering::greater)
		{
			prevNode = node;
			node = node->childGreater;
		}
		else
		{
			prevNode = node;
			node = node->childLesser;
		}
	}

	return {node, prevNode};
}

/// @brief Gets a reference to a loaded message with the given GUID.
/// @param guid GUID of message.
/// @return Message reference, if found, otherwise `nullptr`.
MessageReference *MessageManager::getMessageReference(Guid const &guid) const
{
	MessageManager::Node *node = this->findNode(guid).first;
	if (node == nullptr)
	{
		return nullptr;
	}

	return &node->messageReference;
}

bool MessageManager::createAndLoadMessages(std::map<Guid, std::map<via::Language, char16_t const *>> messages, via::Language fillRemainingWithLanguage) const
{
	// Check none of the GUIDs already in use
	std::size_t numAlreadyInUse = 0;
	for (auto pair : messages)
	{
		Guid guid = pair.first;
		MessageManager::Node *node;
		MessageManager::Node *prevNode;
		std::tie(node, prevNode) = this->findNode(guid);

		if (node != nullptr)
		{
			OutputDebugString(std::format(
								  L"Message with GUID %s already exists",
								  guid.toString())
								  .c_str());
			numAlreadyInUse++;
		}
	}
	if (numAlreadyInUse > 0)
	{
		return false;
	}

	// Initialize languages array once
	static std::uint32_t *s_languages = nullptr;
	if (s_languages == nullptr)
	{
		std::uint32_t *languages = (std::uint32_t *)calloc(1, sizeof(std::uint32_t) * std::to_underlying(via::Language::Max));
		for (std::size_t i = 0; i < std::to_underlying(via::Language::Max); i++)
		{
			languages[i] = i;
		}
		s_languages = languages;
	}

	// Allocate message file
	MessageFile *msgFile = (MessageFile *)calloc(1, sizeof(MessageFile) + sizeof(MessageFileEntry *) * messages.size());
	msgFile->languages = s_languages;
	msgFile->languageCount = std::to_underlying(via::Language::Max);
	msgFile->entryCount = messages.size();

	// Allocate messages and create nodes
	std::size_t entryIdx = 0;
	for (auto pair : messages)
	{
		Guid guid = pair.first;
		auto languageStrings = pair.second;

		MessageFileEntry *entry = (MessageFileEntry *)calloc(1, sizeof(MessageFileEntry) + sizeof(char16_t *) * std::to_underlying(via::Language::Max));
		entry->guid = guid;

		msgFile->entries[entryIdx] = entry;

		MessageManager::Node *node;
		MessageManager::Node *prevNode;
		std::tie(node, prevNode) = this->findNode(guid);

		node = (MessageManager::Node *)calloc(1, sizeof(MessageManager::Node));
		node->parent = prevNode;
		node->childGreater = (MessageManager::Node *)this->nativeObject->messageTree;
		node->childLesser = (MessageManager::Node *)this->nativeObject->messageTree;
		node->guid = guid;
		node->messageReference = MessageReference(msgFile, entryIdx);

		if (guid < prevNode->guid)
		{
			prevNode->childLesser = node;
		}
		else
		{
			prevNode->childGreater = node;
		}

		if (guid < this->nativeObject->messageTree->minChild->guid)
		{
			this->nativeObject->messageTree->minChild = node;
		}
		if (guid > this->nativeObject->messageTree->maxChild->guid)
		{
			this->nativeObject->messageTree->maxChild = node;
		}

		if (languageStrings.contains(fillRemainingWithLanguage))
		{
			node->messageReference.setStringAllLanguages(languageStrings[fillRemainingWithLanguage]);
		}
		for (auto it = languageStrings.begin(); it != languageStrings.end(); ++it)
		{
			node->messageReference.setString(it->first, it->second);
		}

		entryIdx++;
	}

	return true;
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
