#include <cassert>
#include <cstdbool>
#include <cstdint>
#include <cstring>
#include <format>
#include <optional>
#include <stdfloat>
#include <string_view>

#include <reframework/API.hpp>

#include "Guid.hpp"
#include "REFrameworkHelper.hpp"

using namespace REFrameworkHelper;

template <typename T>
static void setFieldPointer(T *pointer, T value)
{
	*pointer = value;
}

template <>
static void setFieldPointer<Object>(Object *pointer, Object value)
{
	// pointer is not really an Object *
	Object oldValue = Object(*(void **)pointer);
	*pointer = value;

	// Update reference count for new value
	if (value != nullptr)
	{
		value.m_object->add_ref();
	}

	// Update reference count for old value
	if (oldValue != nullptr)
	{
		oldValue.m_object->release();
	}
}

template <>
static void setFieldPointer<void *>(void **pointer, void *value)
{
	// Assume for now all void * are managed objects
	setFieldPointer((Object *)pointer, (Object)value);
}

/// @brief Uninstall this hook; invalidates the hook reference.
void HookRef::unhook()
{
	if (m_method != nullptr)
	{
		m_method->remove_hook(m_hookId);
	}
	*this = HookRef();
}

template <typename T>
T Object::getField(std::string_view fieldName, T reframework::InvokeRet::*invokeRetField, bool isValueType) const
{
	assert(m_object != nullptr);

	reframework::API::TypeDefinition *type = m_object->get_type_definition();
	assert(type != nullptr);

	// Get field via getter
	reframework::API::Method *getter = type->find_method(
		std::format("get_{}", fieldName));
	if (getter != nullptr)
	{
		reframework::InvokeRet ret = getter->invoke(m_object, std::span<void *>());

		return ret.*invokeRetField;
	}

	// Get field directly
	T *fieldPtr = m_object->get_field<T>(fieldName, isValueType);
	if (fieldPtr != nullptr)
	{
		return *fieldPtr;
	}

	auto &api = reframework::API::get();
	api->log_error("get: unknown field {}.{}", type->get_full_name(), fieldName);
	assert(0);

	return T();
}

template <typename T>
void Object::setField(std::string_view fieldName, T value, bool isValueType) const
{
	assert(m_object != nullptr);

	reframework::API::TypeDefinition *type = m_object->get_type_definition();
	assert(type != nullptr);

	// Set field via setter
	std::string funcName = std::format("set_{}", fieldName);
	reframework::API::Method *setter = type->find_method(funcName);
	if (setter != nullptr)
	{
		setter->invoke(m_object, {toCallArg<T>(value)});
		return;
	}

	// Set field directly
	T *fieldPtr = m_object->get_field<T>(fieldName, isValueType);
	if (fieldPtr != nullptr)
	{
		setFieldPointer(fieldPtr, value);
		return;
	}

	// Check if field has a getter, use this to determine if field does not exist or is read-only
	auto &api = reframework::API::get();
	funcName[0] = 'g'; // set to get
	reframework::API::Method *getter = type->find_method(funcName);
	if (getter != nullptr)
	{
		api->log_error("set: field {}.{} is read-only", type->get_full_name(), fieldName);
	}
	else
	{
		api->log_error("set: unknown field {}.{}", type->get_full_name(), fieldName);
	}
	assert(0);
}

template <typename T>
T Object::getArray(size_t idx, T reframework::InvokeRet::*invokeRetField) const
{
	assert(m_object != nullptr);

	reframework::API::TypeDefinition *type = m_object->get_type_definition();
	assert(type != nullptr);

	// Get array item via getter
	reframework::API::Method *getter = type->find_method("get_Item");
	if (getter != nullptr)
	{
		reframework::InvokeRet ret = getter->invoke(m_object, {(void *)(uintptr_t)idx});

		return ret.*invokeRetField;
	}

	auto &api = reframework::API::get();
	api->log_error("get: {} is not an array", type->get_full_name());
	assert(0);

	return T();
}

template <typename T>
void Object::setArray(size_t idx, T value) const
{
	assert(m_object != nullptr);

	reframework::API::TypeDefinition *type = m_object->get_type_definition();
	assert(type != nullptr);

	// Set array field via setter
	reframework::API::Method *setter = type->find_method("set_Item");
	if (setter != nullptr)
	{
		setter->invoke(m_object, {(void *)(uintptr_t)idx, toCallArg<T>(value)});
		return;
	}

	auto &api = reframework::API::get();
	api->log_error("set: {} is not an array", type->get_full_name());
	assert(0);
}

/// @brief Get value of field in this type
/// @tparam T Type of value returned
/// @param fieldName Name of field
/// @param isValueType true iff this type is a value type
/// @return Value of field
template <typename T>
T Type::get(std::string_view fieldName, bool isValueType) const
{
	assert(m_type != nullptr);

	// Get field directly
	reframework::API::Field *fieldPtr = m_type->find_field(fieldName);
	if (fieldPtr != nullptr)
	{
		return *(T *)fieldPtr->get_data_raw(nullptr, isValueType);
	}

	auto &api = reframework::API::get();
	api->log_error("get: unknown field {}.{}", m_type->get_full_name(), fieldName);
	assert(0);

	return T();
}

template <typename T>
void Type::set(std::string_view fieldName, T value, bool isValueType) const
{
	assert(m_type != nullptr);

	// Set field directly
	reframework::API::Field *fieldPtr = m_type->find_field(fieldName);
	if (fieldPtr != nullptr)
	{
		setFieldPointer((T *)fieldPtr->get_data_raw(nullptr, isValueType), value);
		return;
	}
	auto &api = reframework::API::get();
	api->log_error("set: unknown field {}.{}", m_type->get_full_name(), fieldName);
	assert(0);
}

/// @brief Helper function to hook TDB function
/// @param fullName Fully qualified name of function e.g. namespace.namespace.class.function()
/// @param preFn Pre-function hook callback
/// @param postFn Post-function hook callback
/// @param ignoreJmp Skips trying to follow the first jmp in the function
/// @return Hook reference, can be used to unhook later
HookRef REFrameworkHelper::hook(std::string_view fullName, REFPreHookFn preFn, REFPostHookFn postFn, bool ignoreJmp)
{
	auto &api = reframework::API::get();
	auto tdb = api->tdb();

	size_t parenPos = fullName.find_first_of('(');
	size_t dotPos = fullName.find_last_of('.', parenPos);
	assert(dotPos != std::string::npos);

	// Fix for function names starting with . (e.g. .ctor())
	while (dotPos > 0 && fullName[dotPos - 1] == '.')
	{
		dotPos -= 1;
	}

	// Workaround for bug in RETypeDB::find_type() where it uses unbounded .data() on the input
	std::string typeName = std::string(fullName.substr(0, dotPos));
	std::string_view funcName = fullName.substr(dotPos + 1);

	reframework::API::Method *method = tdb->find_method(typeName, funcName);
	if (method == nullptr)
	{
		api->log_error("hook: unknown function {}.{}", typeName, funcName);
		assert(0);
		return HookRef();
	}

	int hookId = method->add_hook(preFn, postFn, ignoreJmp);

	return HookRef(method, hookId);
}

/// @brief Helper function to get wrapped singleton
/// @param name Name of singleton
/// @return Wrapped singleton
Object REFrameworkHelper::getSingleton(std::string_view name)
{
	auto &api = reframework::API::get();

	reframework::API::ManagedObject *object = api->get_managed_singleton(name);
	if (object == nullptr)
	{
		api->log_error("getSingleton: unknown singleton {}", name);
		assert(0);
		return Object();
	}

	return Object(object);
}

/// @brief Helper function to get native singleton
/// @param name Name of singleton
/// @return Native singleton
void *REFrameworkHelper::getNativeSingleton(std::string_view name)
{
	auto &api = reframework::API::get();

	void *object = api->get_native_singleton(name);
	if (object == nullptr)
	{
		api->log_error("getNativeSingleton: unknown native singleton {}", name);
		assert(0);
		return nullptr;
	}

	return object;
}

/// @brief Get type by name
/// @param name Name of field
/// @return Type definition
Type REFrameworkHelper::getType(std::string_view name)
{
	auto &api = reframework::API::get();
	auto tdb = api->tdb();

	reframework::API::TypeDefinition *type = tdb->find_type(name);
	if (type == nullptr)
	{
		api->log_error("getType: unknown type {}", name);
		assert(0);
		return Type();
	}

	return Type(type);
}

/// @brief Helper function to get static field of type
/// @tparam T Type of value returned
/// @param fullName Fully qualified name of field, e.g. namespace.namespace.type.field
/// @return Value of field
template <typename T>
T REFrameworkHelper::getStaticField(std::string_view fullName)
{
	size_t dotPos = fullName.find_last_of('.');
	assert(dotPos != std::string::npos);

	// Workaround for bug in RETypeDB::find_type() where it uses unbounded .data() on the input
	std::string typeName = std::string(fullName.substr(0, dotPos));
	std::string_view fieldName = fullName.substr(dotPos + 1);

	Type type = getType(typeName);
	return type.get<T>(fieldName);
}

/// @brief Helper function to create object of given type
/// @param typeName Fully qualified name of type, e.g. namespace.namespace.type
/// @return Created object
Object REFrameworkHelper::createObject(std::string_view typeName)
{
	auto &api = reframework::API::get();
	auto tdb = api->tdb();

	reframework::API::TypeDefinition *type = tdb->find_type(typeName);
	if (type == nullptr)
	{
		api->log_error("createObject: unknown type {}", typeName);
		assert(0);
		return Object();
	}

	reframework::API::ManagedObject *object = type->create_instance();
	assert(object != nullptr);

	return Object(object);
}

/// @brief Helper function to create array of given type and size
/// @param typeName Fully qualified name of type, e.g. namespace.namespace.type
/// @param size Size of array
/// @return Created array
Object REFrameworkHelper::createArray(std::string_view typeName, size_t size)
{
	auto &api = reframework::API::get();
	auto tdb = api->tdb();

	reframework::API::TypeDefinition *type = tdb->find_type(typeName);
	if (type == nullptr)
	{
		api->log_error("createArray: unknown type {}", typeName);
		assert(0);
		return Object();
	}

	reframework::API::ManagedObject *object = api->create_managed_array(type, size);
	assert(object != nullptr);

	return Object(object);
}

/// @brief Helper function to create managed string
/// @param string String value
/// @return Created string
Object REFrameworkHelper::createString(wchar_t const *string)
{
	auto &api = reframework::API::get();

	reframework::API::ManagedObject *object = api->create_managed_string(string);
	assert(object != nullptr);

	return Object(object);
}

#define CREATE_FIELD_GETTER(externalType, invokeRetField)                                                 \
	template <>                                                                                           \
	externalType Object::get(std::string_view fieldName, bool isValueType) const                          \
	{                                                                                                     \
		return externalType(getField<decltype(invokeRetField)>(fieldName, &invokeRetField, isValueType)); \
	}

#define CREATE_FIELD_SETTER(externalType, invokeRetField, valueConverted)                                       \
	template <>                                                                                                 \
	void Object::set(std::string_view fieldName, externalType value, bool isValueType) const                    \
	{                                                                                                           \
		setField<decltype(invokeRetField)>(fieldName, (decltype(invokeRetField))(valueConverted), isValueType); \
	}

#define CREATE_ARRAY_GETTER(externalType, invokeRetField)                              \
	template <>                                                                        \
	externalType Object::get(size_t idx) const                                         \
	{                                                                                  \
		return externalType(getArray<decltype(invokeRetField)>(idx, &invokeRetField)); \
	}

#define CREATE_ARRAY_SETTER(externalType, invokeRetField, valueConverted)                    \
	template <>                                                                              \
	void Object::set(size_t idx, externalType value) const                                   \
	{                                                                                        \
		setArray<decltype(invokeRetField)>(idx, (decltype(invokeRetField))(valueConverted)); \
	}

#define CREATE_TYPE_FIELD_GETTER(externalType) \
	template externalType Type::get(std::string_view fieldName, bool isValueType) const;

#define CREATE_TYPE_FIELD_SETTER(externalType) \
	template void Type::set(std::string_view fieldName, externalType value, bool isValueType) const;

#define CREATE_STATIC_FIELD_GETTER(externalType) \
	template externalType REFrameworkHelper::getStaticField(std::string_view fullName);

#define CREATE_ALL(externalType, invokeRetField, valueConverted)      \
	CREATE_FIELD_GETTER(externalType, invokeRetField)                 \
	CREATE_FIELD_SETTER(externalType, invokeRetField, valueConverted) \
	CREATE_ARRAY_GETTER(externalType, invokeRetField)                 \
	CREATE_ARRAY_SETTER(externalType, invokeRetField, valueConverted) \
	CREATE_TYPE_FIELD_GETTER(externalType)                            \
	CREATE_TYPE_FIELD_SETTER(externalType)                            \
	CREATE_STATIC_FIELD_GETTER(externalType)

CREATE_ALL(bool, reframework::InvokeRet::byte, value)
CREATE_ALL(std::int8_t, reframework::InvokeRet::byte, value)
CREATE_ALL(std::uint8_t, reframework::InvokeRet::byte, value)
CREATE_ALL(std::int16_t, reframework::InvokeRet::word, value)
CREATE_ALL(std::uint16_t, reframework::InvokeRet::word, value)
CREATE_ALL(std::int32_t, reframework::InvokeRet::dword, value)
CREATE_ALL(std::uint32_t, reframework::InvokeRet::dword, value)
CREATE_ALL(std::int64_t, reframework::InvokeRet::qword, value)
CREATE_ALL(std::uint64_t, reframework::InvokeRet::qword, value)
CREATE_ALL(float, reframework::InvokeRet::f, value)
CREATE_ALL(double, reframework::InvokeRet::d, value)
CREATE_ALL(Object, reframework::InvokeRet::ptr, value.m_object)
CREATE_ALL(Guid, reframework::InvokeRet::bytes, value)
