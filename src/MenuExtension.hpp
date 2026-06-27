#include <generator>
#include <iterator>
#include <memory>
#include <span>
#include <vector>

#include "MenuItem.hpp"
#include "REFrameworkHelper.hpp"

template <typename T>
class MenuExtension
{
public:
	/// @brief Create new menu extension.
	/// @param newMenuItems New menu items which are part of this extension.
	MenuExtension(std::span<std::shared_ptr<MenuItem>> newMenuItems)
	{
		m_newMenuItems.insert(
			m_newMenuItems.end(),
			std::make_move_iterator(newMenuItems.begin()),
			std::make_move_iterator(newMenuItems.end()));

		// Install hooks if this is the first instance
		if (s_instances.empty())
		{
			MenuExtension<T>::installHooks();
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
			MenuExtension<T>::uninstallHooks();
		}
	}

protected:
	/// @brief  New menu items added by this menu extension instance.
	std::vector<std::shared_ptr<MenuItem>> m_newMenuItems;

	/// @brief  Get all new menu items across all menu extension instances of
	///         this type.
	/// @return Generator of new menu items.
	static std::generator<MenuItem &> getAllNewMenuItems()
	{
		for (MenuExtension const *instance : s_instances)
		{
			for (std::shared_ptr<MenuItem> const &menuItem : instance->m_newMenuItems)
			{
				co_yield *menuItem;
			}
		}
	}

private:
	/// @brief  Tracks all instances of this type of menu extension.
	static inline std::vector<MenuExtension const *> s_instances;

	// Static state
	static inline bool s_doUpdate = true;

	// Hooks
	static inline std::vector<REFrameworkHelper::HookRef> s_hooks;

	/// @brief Install all hooks used for base menu extensions
	static void installHooks()
	{
		s_hooks.emplace_back(REFrameworkHelper::hook(
			"app.GUILauncherOption.update()",
			[](auto...)
			{
				// call onUpdate() on all menu items
				bool doUpdate = true;
				for (MenuItem &menuItem : getAllNewMenuItems())
				{
					doUpdate = menuItem.onUpdate() && doUpdate;
				}
				s_doUpdate = doUpdate;

				if (!doUpdate)
				{
					return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
				}
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			},
			nullptr));

		s_hooks.emplace_back(REFrameworkHelper::hook(
			"app.GUILauncherOption.lateUpdate()",
			[](auto...)
			{
				if (!s_doUpdate)
				{
					return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
				}
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			},
			nullptr));
	}

	/// @brief Uninstall all hooks used for base menu extensions
	static void uninstallHooks()
	{
		for (REFrameworkHelper::HookRef &hook : s_hooks)
		{
			hook.unhook();
		}
		s_hooks.clear();
	}
};
