#pragma once

#include <Enums_Internal.hpp>

#include <cassert>
#include <map>
#include <string>
#include <uchar.h>

#include "Guid.hpp"
#include "MessageFile.hpp"
#include "MessageReference.hpp"

class MessageManager
{
	struct NativeObject;

private:
	MessageManager::NativeObject *nativeObject;

public:
	MessageManager() = delete;
	MessageManager(void *nativeObject)
		: nativeObject((MessageManager::NativeObject *)nativeObject)
	{
		assert(nativeObject != nullptr);
	};

	MessageReference *getMessageReference(Guid const &guid) const;
	char16_t *getString(Guid const &guid, via::Language language) const;

	bool replaceMessageByName(std::string name, std::map<via::Language, char16_t const *> languageStrings, via::Language fillRemainingWithLanguage = via::Language::Unknown) const;
};
