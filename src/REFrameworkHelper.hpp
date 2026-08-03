#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <reframework/API.hpp>

namespace REFrameworkHelper
{
	typedef decltype(reframework::InvokeRet::bytes) ValueTypeArray;
	struct Object;
	struct Type;

	template <typename T>
	void *toCallArg(T arg)
	{
		if constexpr (std::is_convertible_v<std::decay_t<T>, Object>)
		{
			return ((Object)arg).m_object;
		}
		else if constexpr (std::is_same_v<std::decay_t<T>, ValueTypeArray>)
		{
			return arg.data();
		}
		else
		{
			void *buf = 0;
			static_assert(sizeof(arg) <= sizeof(buf));
			memcpy_s(&buf, sizeof(buf), &arg, sizeof(arg));
			return buf;
		}
	}

	/// @brief Handle for TDB method hooks
	struct HookRef
	{
		reframework::API::Method *m_method;
		int m_hookId;

		bool valid() const { return m_method != nullptr && m_hookId != -1; }

		HookRef() : HookRef(nullptr, -1) {}
		HookRef(reframework::API::Method *method, int hookId)
			: m_method(method), m_hookId(hookId) {}

		void unhook();
	};

	/// @brief Wrapper for reframework::API::ManagedObject
	struct Object
	{
		template <typename T>
		struct ObjectGetterSetter
		{
		public:
			Object m_obj;
			std::variant<std::size_t, std::string_view> m_key;

			ObjectGetterSetter(Object obj, std::size_t idx) : m_obj(obj), m_key(idx) {}

			// Constructor for name
			ObjectGetterSetter(Object obj, std::string_view name) : m_obj(obj), m_key(name) {}

			operator T() const
			{
				return getInternal();
			}

			T operator=(T value) const
			{
				setInternal(value);
				return value;
			}

			const ObjectGetterSetter &operator=(const ObjectGetterSetter &value) const
			{
				setInternal(value.getInternal());
				return value;
			}

			bool operator==(T const &b) const
			{
				return getInternal() == b;
			}

			template <typename TInner = Object>
			TInner get(std::string_view fieldName, bool isValueType = false) const
			{
				return getInternal().get<TInner>(fieldName, isValueType);
			}

			template <typename TInner = Object>
			void set(std::string_view fieldName, TInner value, bool isValueType = false) const
			{
				getInternal().set<TInner>(fieldName, value, isValueType);
			}

			template <typename TInner = Object>
			TInner get(std::size_t idx) const
			{
				return getInternal().get<TInner>(idx);
			}

			template <typename TInner = Object>
			void set(std::size_t idx, TInner value) const
			{
				getInternal().set<TInner>(idx, value);
			}

			template <typename TInner = Object, class Enum>
				requires std::is_enum_v<Enum>
			TInner get(Enum e) const
			{
				return getInternal().get<TInner>(e);
			}

			template <typename TInner = Object, class Enum>
				requires std::is_enum_v<Enum>
			void set(Enum e, TInner value) const
			{
				getInternal().set<TInner>(e, value);
			}

			template <typename TInner = Object>
			ObjectGetterSetter<TInner> operator[](std::string_view name)
			{
				return ObjectGetterSetter<TInner>(getInternal(), name);
			}

			template <typename TInner = Object>
			ObjectGetterSetter<TInner> operator[](std::size_t idx)
			{
				return ObjectGetterSetter<TInner>(getInternal(), idx);
			}

			template <typename TInner = Object, class Enum>
				requires std::is_enum_v<Enum>
			ObjectGetterSetter<TInner> operator[](Enum e)
			{
				return ObjectGetterSetter<TInner>(getInternal(), std::to_underlying(e));
			}

			template <typename TInner = Object>
			const TInner operator[](std::string_view name) const
			{
				return getInternal().get<TInner>(name);
			}

			template <typename TInner = Object>
			const TInner operator[](std::size_t idx) const
			{
				return getInternal().get<TInner>(idx);
			}

			template <typename TInner = Object, class Enum>
				requires std::is_enum_v<Enum>
			const TInner operator[](Enum e) const
			{
				return getInternal().get<TInner>(std::to_underlying(e));
			}

			template <typename TInner = void, typename... TArgs>
			TInner call(std::string_view funcName, TArgs... args) const
			{
				return getInternal().call<TInner>(funcName, args...);
			}

		protected:
			T getInternal() const
			{
				return std::visit(
					[this](auto &&key)
					{
						return m_obj.get<T>(key);
					},
					m_key);
			}

			void setInternal(T value) const
			{
				std::visit(
					[this, value](auto &&key)
					{
						m_obj.set<T>(key, value);
					},
					m_key);
			}
		};

		reframework::API::ManagedObject *m_object;

		Object() : Object(nullptr) {}
		Object(const void *object) : m_object((reframework::API::ManagedObject *)object) {}

		reframework::API::ManagedObject *operator=(void *value)
		{
			m_object = (reframework::API::ManagedObject *)value;
			return m_object;
		}

		template <typename T = Object>
		T get(std::string_view fieldName, bool isValueType = false) const;

		template <typename T = Object>
		void set(std::string_view fieldName, T value, bool isValueType = false) const;

		template <typename T = Object>
		T get(std::size_t idx) const;

		template <typename T = Object>
		void set(std::size_t idx, T value) const;

		template <typename T = Object, class Enum>
			requires std::is_enum_v<Enum>
		T get(Enum e) const
		{
			return this->get(std::to_underlying(e));
		}

		template <typename T = Object, class Enum>
			requires std::is_enum_v<Enum>
		void set(Enum e, T value) const
		{
			this->set(std::to_underlying(e), value);
		}

		template <typename T = Object>
		ObjectGetterSetter<T> operator[](std::string_view name)
		{
			return ObjectGetterSetter<T>(*this, name);
		}

		template <typename T = Object>
		ObjectGetterSetter<T> operator[](std::size_t idx)
		{
			return ObjectGetterSetter<T>(*this, idx);
		}

		template <typename T = Object, class Enum>
			requires std::is_enum_v<Enum>
		ObjectGetterSetter<T> operator[](Enum e)
		{
			return ObjectGetterSetter<T>(*this, std::to_underlying(e));
		}

		template <typename T = Object>
		const T operator[](std::string_view name) const
		{
			return this->get<T>(name);
		}

		template <typename T = Object>
		const T operator[](std::size_t idx) const
		{
			return this->get<T>(idx);
		}

		template <typename T = Object, class Enum>
			requires std::is_enum_v<Enum>
		const T operator[](Enum e) const
		{
			return this->get<T>(std::to_underlying(e));
		}

		template <typename T = void, typename... TArgs>
		T call(std::string_view funcName, TArgs... args) const
		{
			assert(this->m_object != nullptr);
			auto &api = reframework::API::get();

			reframework::API::TypeDefinition *type = this->m_object->get_type_definition();
			assert(type != nullptr);

			reframework::API::Method *function = type->find_method(funcName);
			if (function == nullptr)
			{
				api->log_error("call: unknown function {}.{}", type->get_full_name(), funcName);
				assert(0);
				return T();
			}
			if (function->is_static())
			{
				api->log_error("call: function {}.{} is static", type->get_full_name(), funcName);
				assert(0);
				return T();
			}

			// Call function
			std::array<void *, sizeof...(args)> argv = {toCallArg(args)...};
			reframework::InvokeRet ret = function->invoke(this->m_object, argv);
			if constexpr (std::is_void_v<T>)
			{
				return;
			}
			else
			{
				T result{};
				std::memcpy(&result, &ret, sizeof(T));
				return result;
			}
		}

		operator void *() const
		{
			return this->m_object;
		}

	private:
		template <typename T>
		T getField(std::string_view fieldName, T reframework::InvokeRet::*invokeRetField, bool isValueType = false) const;

		template <typename T>
		void setField(std::string_view fieldName, T value, bool isValueType = false) const;

		template <typename T>
		T getArray(std::size_t idx, T reframework::InvokeRet::*invokeRetField) const;

		template <typename T>
		void setArray(std::size_t idx, T value) const;
	};

	/// @brief Wrapper for reframework::API::TypeDefinition
	struct Type
	{
		reframework::API::TypeDefinition *m_type;

		Type() : Type((reframework::API::TypeDefinition *)nullptr) {}
		Type(void *type) : Type((reframework::API::TypeDefinition *)type) {}
		Type(reframework::API::TypeDefinition *type)
			: m_type(type) {}

		template <typename T = Object>
		const T operator[](std::string_view name) const
		{
			return this->get<T>(name);
		}

		template <typename T = Object>
		T get(std::string_view fieldName, bool isValueType = false) const;

		template <typename T = Object>
		void set(std::string_view fieldName, T value, bool isValueType = false) const;

		template <typename T = void, typename... TArgs>
		T call(std::string_view funcName, TArgs... args) const
		{
			assert(this->m_type != nullptr);
			auto &api = reframework::API::get();

			// Call function
			reframework::API::Method *function = m_type->find_method(funcName);
			if (function == nullptr)
			{
				api->log_error("call: unknown function {}.{}", m_type->get_full_name(), funcName);
				assert(0);
				return T();
			}
			if (!function->is_static())
			{
				api->log_error("call: function {}.{} is not static", m_type->get_full_name(), funcName);
				assert(0);
				return T();
			}

			// Call function
			if constexpr (std::is_same_v<std::decay_t<Object>, T>)
			{
				return Object(function->call<void *>(api->get_vm_context(), toCallArg(args)...));
			}
			else
			{
				return function->call<T>(api->get_vm_context(), toCallArg(args)...);
			}
		}
	};

	HookRef hook(std::string_view fullName, REFPreHookFn pre_fn, REFPostHookFn post_fn, bool ignoreJmp = false);

	Object getSingleton(std::string_view name);

	void *getNativeSingleton(std::string_view name);

	Type getType(std::string_view name);

	template <typename T>
	T getStaticField(std::string_view fullName);

	Object createObject(std::string_view typeName);

	Object createArray(std::string_view typeName, std::size_t size);

	Object createString(wchar_t const *string);
};
