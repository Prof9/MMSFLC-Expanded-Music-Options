#include <Enums_Internal.hpp>

#include <cstddef>

#include "Guid.hpp"
#include "MessageFile.hpp"
#include "MessageReference.hpp"

/// @brief Create new reference to message within this message file.
/// @param guid GUID of message, must be in this message file.
/// @return Reference containing pointer to message file (i.e. `this`), and message index.
MessageReference MessageFile::createMessageReference(Guid const &guid)
{
	for (std::size_t i = 0; i < this->entryCount; ++i)
	{
		if (this->entries[i]->guid == guid)
		{
			return MessageReference(this, i);
		}
	}
	return MessageReference();
}
