#pragma once

#include <Enums_Internal.hpp>

#include <cassert>
#include <map>
#include <string>
#include <uchar.h>
#include <utility>

#include "Guid.hpp"
#include "MessageFile.hpp"
#include "MessageReference.hpp"

class MessageManager
{
	struct NativeObject;
	struct Tree;
	struct Node;
	struct NodeContainer;

private:
	MessageManager::NativeObject *nativeObject;

	std::pair<MessageManager::Node *, MessageManager::Node *> findNode(Guid const &guid) const;
	MessageReference *getMessageReference(Guid const &guid) const;

public:
	MessageManager() = delete;
	MessageManager(void *nativeObject)
		: nativeObject((MessageManager::NativeObject *)nativeObject)
	{
		assert(nativeObject != nullptr);
	};

	bool createAndLoadMessages(std::map<Guid, std::map<via::Language, char16_t const *>> messages, via::Language fillRemainingWithLanguage = via::Language::Unknown) const;
	bool replaceMessageByName(std::string_view name, std::map<via::Language, char16_t const *> languageStrings, via::Language fillRemainingWithLanguage = via::Language::Unknown) const;
};
