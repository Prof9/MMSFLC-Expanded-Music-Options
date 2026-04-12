#pragma once

#include <Enums_Internal.hpp>

#include <map>
#include <string>

#include "Guid.hpp"

class MessageUtility
{
public:
	static Guid *getMessageGuidByName(std::string name);
};
