#include <generator>
#include <iterator>
#include <span>
#include <vector>

#include "MenuItem.hpp"

template <typename T>
class MenuExtension
{
public:
	/// @brief Create new menu extension.
	/// @param newMenuItems New menu items which are part of this extension.
	MenuExtension(std::span<MenuItem const> newMenuItems)
	{
		m_newMenuItems.append_range(newMenuItems);

		// Install hooks if this is the first instance
		if (s_instances.empty())
		{
			T::installHooks();
		}
		s_instances.push_back(this);
	}

	~MenuExtension()
	{
		// Uninstall hooks if this is the last instance
		s_instances.erase(std::remove(s_instances.begin(), s_instances.end(), this));
		if (s_instances.empty())
		{
			T::uninstallHooks();
		}
	}

protected:
	/// @brief  New menu items added by this menu extension instance.
	std::vector<MenuItem> m_newMenuItems;

	/// @brief  Get all new menu items across all menu extension instances of
	///         this type.
	/// @return Generator of new menu items.
	static std::generator<MenuItem const *> getAllNewMenuItems()
	{
		for (MenuExtension const *instance : s_instances)
		{
			for (MenuItem const menuItem : instance->m_newMenuItems)
			{
				co_yield &menuItem;
			}
		}
	}

private:
	/// @brief  Tracks all instances of this type of menu extension.
	static inline std::vector<MenuExtension const *> s_instances;
};
