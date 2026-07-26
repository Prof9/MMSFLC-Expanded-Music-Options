#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

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
		template <typename T>
		struct ObjectGetterSetterByIndex;
		template <typename T>
		struct ObjectGetterSetterByName;

		template <typename T>
		struct ObjectGetterSetter
		{
		public:
			Object m_obj;

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
				return getInternal.get<TInner>(e);
			}

			template <typename TInner = Object, class Enum>
				requires std::is_enum_v<Enum>
			void set(Enum e, TInner value) const
			{
				getInternal().set<TInner>(e, value);
			}

			template <typename TInner = Object>
			ObjectGetterSetterByName<TInner> operator[](std::string_view name)
			{
				return ObjectGetterSetterByName<TInner>(getInternal(), name);
			}

			template <typename TInner = Object>
			ObjectGetterSetterByIndex<TInner> operator[](std::size_t idx)
			{
				return ObjectGetterSetterByIndex<TInner>(getInternal(), idx);
			}

			template <typename TInner = Object, class Enum>
				requires std::is_enum_v<Enum>
			ObjectGetterSetterByIndex<TInner> operator[](Enum e)
			{
				return ObjectGetterSetterByIndex<TInner>(getInternal(), std::to_underlying(e));
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
			ObjectGetterSetter(Object obj) : m_obj(obj) {}

			virtual T getInternal() const = 0;
			virtual void setInternal(T value) const = 0;
		};

		template <typename T>
		struct ObjectGetterSetterByIndex : ObjectGetterSetter<T>
		{
			ObjectGetterSetterByIndex(Object obj, std::size_t index) : ObjectGetterSetter<T>(obj), m_index(index) {}

			T operator=(T value) const
			{
				return ObjectGetterSetter<T>::operator=(value);
			}

		protected:
			std::size_t m_index;

			T getInternal() const
			{
				return this->m_obj.get<T>(m_index);
			}

			void setInternal(T value) const
			{
				this->m_obj.set<T>(m_index, value);
			}
		};

		template <typename T>
		struct ObjectGetterSetterByName : ObjectGetterSetter<T>
		{
			ObjectGetterSetterByName(Object obj, std::string_view name) : ObjectGetterSetter<T>(obj), m_name(name) {}

			T operator=(T value) const
			{
				return ObjectGetterSetter<T>::operator=(value);
			}

		protected:
			std::string_view m_name;

			T getInternal() const
			{
				return this->m_obj.get<T>(m_name);
			}

			void setInternal(T value) const
			{
				this->m_obj.set<T>(m_name, value);
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
		ObjectGetterSetterByName<T> operator[](std::string_view name)
		{
			return ObjectGetterSetterByName<T>(*this, name);
		}

		template <typename T = Object>
		ObjectGetterSetterByIndex<T> operator[](std::size_t idx)
		{
			return ObjectGetterSetterByIndex<T>(*this, idx);
		}

		template <typename T = Object, class Enum>
			requires std::is_enum_v<Enum>
		ObjectGetterSetterByIndex<T> operator[](Enum e)
		{
			return ObjectGetterSetterByIndex<T>(*this, std::to_underlying(e));
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

			reframework::API::TypeDefinition *type = this->m_object->get_type_definition();
			assert(type != nullptr);

			// Call function
			reframework::API::Method *function = type->find_method(funcName);
			if (function != nullptr)
			{
				std::vector<void *> argv;
				([&]
				 { argv.push_back((void *)args); }(),
				 ...);
				reframework::InvokeRet ret = function->invoke(this->m_object, argv);
				if constexpr (std::is_void_v<T>)
				{
					return;
				}
				else
				{
					return *((T *)&ret);
				}
			}

			auto &api = reframework::API::get();
			api->log_error("call: unknown function {}.{}", type->get_full_name(), funcName);
			assert(0);

			return T();
		}

		operator void *() const { return this->m_object; }

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
			if (function != nullptr)
			{
				return function->call<T>(api->get_vm_context(), args...);
			}

			api->log_error("call: unknown function {}.{}", m_type->get_full_name(), funcName);
			assert(0);

			return T();
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
