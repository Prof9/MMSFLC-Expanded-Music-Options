#include <Enums_Internal.hpp>

#include <cstddef>
#include <uchar.h>
#include <utility>

#include "MessageFile.hpp"
#include "MessageReference.hpp"

/// @brief Resolves message reference to raw string pointer-pointer.
/// @param language String language.
/// @return Pointer-pointer of raw string, if found, otherwise `nullptr`.
char16_t const **MessageReference::getStringPointer(via::Language language) const
{
	if (!this->isValid())
	{
		return nullptr;
	}

	for (std::size_t i = 0; i < this->messageFile->languageCount; ++i)
	{
		if (this->messageFile->languages[i] == std::to_underlying(language))
		{
			return &this->messageFile->entries[this->messageIndex]->strings[i];
		}
	}

	return nullptr;
}

/// @brief Resolves message reference to raw string.
/// @param language String language.
/// @return Pointer to raw string, if found, otherwise `nullptr`.
char16_t const *MessageReference::getString(via::Language language) const
{
	// Checks isValid()
	char16_t const **stringPointer = this->getStringPointer(language);
	if (stringPointer == nullptr)
	{
		return nullptr;
	}

	return *stringPointer;
}

/// @brief Changes string referenced by this message reference for a single language.
/// @param language String language.
/// @param string New string.
/// @return `true` on success, `false` on failure (e.g. message reference is invalid or message file does not contain given language).
bool MessageReference::setString(via::Language language, char16_t const *string) const
{
	// Checks isValid()
	char16_t const **stringPointer = this->getStringPointer(language);
	if (stringPointer == nullptr)
	{
		return false;
	}

	*stringPointer = string;
	return true;
}

/// @brief Changes string referenced by this message reference for all languages in the message file.
/// @param string New string.
/// @return `true` on success, `false` on failure (e.g. message reference is invalid).
bool MessageReference::setStringAllLanguages(char16_t const *string) const
{
	if (!this->isValid())
	{
		return false;
	}

	for (std::size_t i = 0; i < this->messageFile->languageCount; ++i)
	{
		this->messageFile->entries[this->messageIndex]->strings[i] = string;
	}
	return true;
}
