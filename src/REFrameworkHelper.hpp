#pragma once

#include <optional>
#include <string_view>
#include <tuple>

#include <reframework/API.hpp>

namespace REFrameworkHelper
{
	typedef decltype(reframework::InvokeRet::bytes) ValueTypeArray;

	/// @brief Handle for TDB method hooks
	struct HookRef
	{
		reframework::API::Method *m_method;
		int m_hookId;

		bool valid() { return m_method != nullptr && m_hookId != -1; }

		HookRef() : HookRef(nullptr, -1) {}
		HookRef(reframework::API::Method *method, int hookId)
			: m_method(method), m_hookId(hookId) {}

		void unhook();
	};

	/// @brief Wrapper for reframework::API::ManagedObject
	struct Object
	{
		reframework::API::ManagedObject *m_object;

		Object() : Object((reframework::API::ManagedObject *)nullptr) {}
		Object(void *object) : Object((reframework::API::ManagedObject *)object) {}
		Object(reframework::API::ManagedObject *object)
			: m_object(object) {}

		bool null() { return m_object == nullptr; }

		template <typename T>
		T get(std::string_view fieldName, bool isValueType = false);

		template <typename T>
		void set(std::string_view fieldName, T value, bool isValueType = false);

		template <typename T>
		T get(size_t idx);

		template <typename T>
		void set(size_t idx, T value);

		template <typename T>
		T call(std::string_view funcName, std::vector<void *> args = {});

		operator void *() const { return this->m_object; }

	private:
		template <typename T>
		T getField(std::string_view fieldName, T reframework::InvokeRet::*invokeRetField, bool isValueType = false);

		template <typename T>
		void setField(std::string_view fieldName, T value, bool isValueType = false);

		template <typename T>
		T getArray(size_t idx, T reframework::InvokeRet::*invokeRetField);

		template <typename T>
		void setArray(size_t idx, T value);

		template <typename T>
		T callInternal(std::string_view funcName, T reframework::InvokeRet::*invokeRetField, std::vector<void *> args);
	};

	/// @brief Wrapper for reframework::API::TypeDefinition
	struct Type
	{
		reframework::API::TypeDefinition *m_type;

		Type() : Type((reframework::API::TypeDefinition *)nullptr) {}
		Type(void *type) : Type((reframework::API::TypeDefinition *)type) {}
		Type(reframework::API::TypeDefinition *type)
			: m_type(type) {}

		bool null() { m_type == nullptr; }

		template <typename T>
		T get(std::string_view fieldName, bool isValueType = false);
	};

	HookRef hook(std::string_view fullName, REFPreHookFn pre_fn, REFPostHookFn post_fn, bool ignoreJmp = false);

	Object getSingleton(std::string_view name);

	Type getType(std::string_view name);

	template <typename T>
	T getStaticField(std::string_view fullName);

	Object createObject(std::string_view typeName);

	Object createArray(std::string_view typeName, size_t size);

	Object createString(wchar_t const *string);
};
