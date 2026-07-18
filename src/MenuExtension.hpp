#include <generator>
#include <iterator>
#include <memory>
#include <optional>
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
	MenuExtension(std::span<std::shared_ptr<MenuItem>> newMenuItems, std::function<void()> onSaveFunc)
		: m_onSaveFunc(onSaveFunc)
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
	/// @brief New menu items added by this menu extension instance.
	std::vector<std::shared_ptr<MenuItem>> m_newMenuItems;

	/// @brief All active menu items across all instances.
	static inline std::vector<std::shared_ptr<MenuItem>> s_activeMenuItems;

private:
	/// @brief Tracks all instances of this type of menu extension.
	static inline std::vector<MenuExtension const *> s_instances;

	// Instance state
	std::function<void()> m_onSaveFunc = nullptr;

	// Static state
	static inline bool s_doUpdate = true;

	// Hooks
	static inline std::vector<REFrameworkHelper::HookRef> s_hooks;

	/// @brief Install all hooks used for base menu extensions
	static void installHooks()
	{
		s_hooks.emplace_back(REFrameworkHelper::hook(
			"app.GUILauncherOption..ctor()",
			[](auto...)
			{
				// Build list of all valid menu items based on mounted DLC
				REFrameworkHelper::Object dlcContentsManager = REFrameworkHelper::getSingleton("app.DLCContentsManager");
				std::vector<std::shared_ptr<MenuItem>> activeMenuItems;
				for (MenuExtension const *instance : s_instances)
				{
					for (std::shared_ptr<MenuItem> const &menuItem : instance->m_newMenuItems)
					{
						if (menuItem->m_requiredDLC != app::DLCContentsManager::DLC_TYPE::INVALID)
						{
							if (!dlcContentsManager.call<bool>("getHasDLC", {(void *)(intptr_t)std::to_underlying(menuItem->m_requiredDLC)}))
							{
								continue;
							}
						}
						activeMenuItems.push_back(menuItem);
					}
				}
				s_activeMenuItems = activeMenuItems;

				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			},
			nullptr));

		// Hook method which is called each time the Settings menu updates
		s_hooks.emplace_back(REFrameworkHelper::hook(
			"app.GUILauncherOption.update()",
			[](auto...)
			{
				// call onUpdate() on all active menu items
				bool doUpdate = true;
				for (std::shared_ptr<MenuItem> menuItem : s_activeMenuItems)
				{
					doUpdate = menuItem->onUpdate() && doUpdate;
				}
				s_doUpdate = doUpdate;

				if (!doUpdate)
				{
					return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
				}
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			},
			nullptr));

		// Hook method which is called when user exits the Settings menu
		s_hooks.emplace_back(REFrameworkHelper::hook(
			"app.GUILauncherOption.onDestroy()",
			[](auto...)
			{
				for (MenuExtension const *instance : s_instances)
				{
					if (instance->m_onSaveFunc != nullptr)
					{
						instance->m_onSaveFunc();
					}
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
