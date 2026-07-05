#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <tuple>
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

			operator T()
			{
				return getInternal();
			}

			T operator=(T value)
			{
				setInternal(value);
				return value;
			}

			template <typename TInner = Object>
			TInner get(std::string_view fieldName, bool isValueType = false)
			{
				return getInternal().get(fieldName, isValueType);
			}

			template <typename TInner = Object>
			void set(std::string_view fieldName, TInner value, bool isValueType = false)
			{
				getInternal().set(fieldName, value, isValueType);
			}

			template <typename TInner = Object>
			TInner get(std::size_t idx)
			{
				return getInternal().get(idx);
			}

			template <typename TInner = Object>
			void set(std::size_t idx, TInner value)
			{
				getInternal().set(idx, value);
			}

			template <typename TInner = Object, class Enum>
				requires std::is_enum_v<Enum>
			TInner get(Enum e) const
			{
				return getInternal.get(e);
			}

			template <typename TInner = Object, class Enum>
				requires std::is_enum_v<Enum>
			void set(Enum e, TInner value)
			{
				getInternal().set(e, value);
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

			template <typename TInner = void>
			TInner call(std::string_view funcName, std::vector<void *> args = {})
			{
				return getInternal().call<TInner>(funcName, args);
			}

		protected:
			ObjectGetterSetter(Object obj) : m_obj(obj) {}

			virtual T getInternal() = 0;
			virtual void setInternal(T value) = 0;
		};

		template <typename T>
		struct ObjectGetterSetterByIndex : ObjectGetterSetter<T>
		{
			ObjectGetterSetterByIndex(Object obj, std::size_t index) : ObjectGetterSetter<T>(obj), m_index(index) {}

			T operator=(T value)
			{
				return ObjectGetterSetter<T>::operator=(value);
			}

		protected:
			std::size_t m_index;

			T getInternal()
			{
				return this->m_obj.get<T>(m_index);
			}

			void setInternal(T value)
			{
				this->m_obj.set<T>(m_index, value);
			}
		};

		template <typename T>
		struct ObjectGetterSetterByName : ObjectGetterSetter<T>
		{
			ObjectGetterSetterByName(Object obj, std::string_view name) : ObjectGetterSetter<T>(obj), m_name(name) {}

			T operator=(T value)
			{
				return ObjectGetterSetter<T>::operator=(value);
			}

		protected:
			std::string_view m_name;

			T getInternal()
			{
				return this->m_obj.get<T>(m_name);
			}

			void setInternal(T value)
			{
				this->m_obj.set<T>(m_name, value);
			}
		};

		reframework::API::ManagedObject *m_object;

		Object() : Object((reframework::API::ManagedObject *)nullptr) {}
		Object(const void *object) : Object((reframework::API::ManagedObject *)object) {}
		Object(reframework::API::ManagedObject *object)
			: m_object(object) {}

		reframework::API::ManagedObject *operator=(void *value)
		{
			m_object = (reframework::API::ManagedObject *)value;
			return m_object;
		}

		template <typename T = Object>
		T get(std::string_view fieldName, bool isValueType = false);

		template <typename T = Object>
		void set(std::string_view fieldName, T value, bool isValueType = false);

		template <typename T = Object>
		T get(std::size_t idx);

		template <typename T = Object>
		void set(std::size_t idx, T value);

		template <typename T = Object, class Enum>
			requires std::is_enum_v<Enum>
		T get(Enum e)
		{
			return this->get(std::to_underlying(e));
		}

		template <typename T = Object, class Enum>
			requires std::is_enum_v<Enum>
		void set(Enum e, T value)
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

		template <typename T = void>
		T call(std::string_view funcName, std::vector<void *> args = {});

		friend bool
		operator==(Object const &a, Object const &b) = default;
		operator void *() const { return this->m_object; }

	private:
		template <typename T>
		T getField(std::string_view fieldName, T reframework::InvokeRet::*invokeRetField, bool isValueType = false);

		template <typename T>
		void setField(std::string_view fieldName, T value, bool isValueType = false);

		template <typename T>
		T getArray(std::size_t idx, T reframework::InvokeRet::*invokeRetField);

		template <typename T>
		void setArray(std::size_t idx, T value);

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

		template <typename T = Object>
		const T operator[](std::string_view name) const
		{
			return this->get<T>(name);
		}

		template <typename T = Object>
		T get(std::string_view fieldName, bool isValueType = false) const;
	};

	HookRef hook(std::string_view fullName, REFPreHookFn pre_fn, REFPostHookFn post_fn, bool ignoreJmp = false);

	Object getSingleton(std::string_view name);

	Type getType(std::string_view name);

	template <typename T>
	T getStaticField(std::string_view fullName);

	Object createObject(std::string_view typeName);

	Object createArray(std::string_view typeName, std::size_t size);

	Object createString(wchar_t const *string);
};
