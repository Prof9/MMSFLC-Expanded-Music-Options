#include <Enums_Internal.hpp>

#include <map>
#include <string>

#include <reframework/API.hpp>

#include "Guid.hpp"
#include "MessageReference.hpp"
#include "MessageUtility.hpp"

/// @brief Get GUID of loaded message by name.
/// @param name Name of message in message file.
/// @return GUID, if found, otherwise `nullptr`.
Guid *MessageUtility::getMessageGuidByName(std::string name)
{
	// Get MsgID once
	static reframework::API::TypeDefinition *msgID = nullptr;
	if (msgID == nullptr)
	{
		msgID = reframework::API::get()->tdb()->find_type("via.gui.MsgID");
		assert(msgID != nullptr);
	}

	reframework::API::Field *field = msgID->find_field(name);
	if (field == nullptr)
	{
		reframework::API::get()->log_error("cannot find message %s", name);
		return nullptr;
	}

	return (Guid *)field->get_data_raw(nullptr, true);
}
