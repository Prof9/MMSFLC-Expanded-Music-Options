#include <Enums_Internal.hpp>

#include <functional>
#include <iterator>
#include <memory>
#include <utility>

#include <reframework/API.hpp>

#include "AudioMenuExtension.hpp"
#include "Guid.hpp"
#include "MenuItem.hpp"
#include "REFrameworkHelper.hpp"

using namespace REFrameworkHelper;

static const ptrdiff_t FLUENT_SCROLL_LIST_OFFSET_BAR_PATH = 0x2D0;

/// @brief Updates list display
/// @param launcherOptionSound app.cLauncherOptionSound object
void AudioMenuExtension::updateList(Object launcherOptionSound)
{
	Object soundList = launcherOptionSound["_SoundList"];
	Object nameList = launcherOptionSound["NameList"];
	Object cursorIndexList = launcherOptionSound["_CursolIndex"];
	Object cursorMaxList = launcherOptionSound["CursolMax"];
	Object cursorNameListList = launcherOptionSound["_CursolNameList"];
	Object volumeInitValueList = launcherOptionSound["VolumeInitValue"];

	Object itemInstTbl = soundList["ItemInstTbl"];
	std::int32_t itemInstTblLength = itemInstTbl.get<std::int32_t>("Length");

	std::uint32_t patternSlider = getStaticField<std::uint32_t>("via.gui.asset.ui00030000._ITM_List_Item_._Pattern_Slider");
	std::uint32_t patternLRSelection = getStaticField<std::uint32_t>("via.gui.asset.ui00030000._ITM_List_Item_._Pattern_LR_Selection");

	// create temporary strings
	Object str_MsgId_Option_List_Item_Name = createString(L"MsgId_Option_List_Item_Name");
	Object str_ObjPath_PNL_Option_List_Item_Slider_Path = createString(L"ObjPath_PNL_Option_List_Item_Slider_Path");
	Object str_ObjPath_PNL_Option_List_Item_LR_Selection_Path = createString(L"ObjPath_PNL_Option_List_Item_LR_Selection_Path");
	Object str_Float_Slider_Value_Init = createString(L"Float_Slider_Value_Init");
	Object str_Float_Slider_Value_Current = createString(L"Float_Slider_Value_Current");
	Object str_MsgId_LR_Selection_Button_Message = createString(L"MsgId_LR_Selection_Button_Message");
	Object str_Bool_LR_Selection_Cursor_Left_Visible = createString(L"Bool_LR_Selection_Cursor_Left_Visible");
	Object str_Bool_LR_Selection_Cursor_Right_Visible = createString(L"Bool_LR_Selection_Cursor_Right_Visible");

	for (std::size_t i = 0; i < itemInstTblLength; ++i)
	{
		Object selectItem = itemInstTbl[i];
		std::int32_t listIndex = selectItem.get<std::int32_t>("ListIndex");

		// Normally the game will compute a MurmurHash3 of the string, but this is easier
		Object parameterVariable = selectItem.call<Object>("getParameterLegacy(System.String)", str_MsgId_Option_List_Item_Name);
		Guid nameGuid = nameList.get<Guid>(listIndex);
		parameterVariable.set<Guid>("ValueMessageId", {nameGuid});

		Object cursorNameList = cursorNameListList[listIndex];
		std::int32_t cursorIndex = cursorIndexList.get<std::int32_t>(listIndex);
		std::int32_t cursorMax = cursorMaxList.get<std::int32_t>(listIndex);

		// assume option is a slider type if it has no name list
		if (cursorNameList == nullptr)
		{
			selectItem.set<std::uint32_t>("StatePattern", patternSlider);

			parameterVariable = selectItem.call<Object>("getParameterLegacy(System.String)", str_ObjPath_PNL_Option_List_Item_Slider_Path);
			Object objPath = parameterVariable["ValueObjectPath"];
			Object sliderPanel = selectItem.call<Object>("getObject(System.String)", objPath);

			Object paramVariable_floatSliderValueInit = sliderPanel.call<Object>("getParameterLegacy(System.String)", str_Float_Slider_Value_Init);
			double volumeInitValue = volumeInitValueList.get<double>(listIndex);
			double sliderValueInit = volumeInitValue / (cursorMax - 1) * 100.0;
			paramVariable_floatSliderValueInit.set<double>("ValueFloat", sliderValueInit);

			Object paramVariable_floatSliderValueCurrent = sliderPanel.call<Object>("getParameterLegacy(System.String)", str_Float_Slider_Value_Current);
			double sliderValueCurrent = (double)cursorIndex / (cursorMax - 1) * 100.0;
			paramVariable_floatSliderValueCurrent.set<double>("ValueFloat", sliderValueCurrent);
		}
		else
		{
			selectItem.set<std::uint32_t>("StatePattern", patternLRSelection);

			parameterVariable = selectItem.call<Object>("getParameterLegacy(System.String)", str_ObjPath_PNL_Option_List_Item_LR_Selection_Path);
			Object objPath = parameterVariable["ValueObjectPath"];
			Object selectionPanel = selectItem.call<Object>("getObject(System.String)", objPath);

			Object parameterVariable_buttonMessage = selectionPanel.call<Object>("getParameterLegacy(System.String)", str_MsgId_LR_Selection_Button_Message);
			Guid cursorGuid = cursorNameList.get<Guid>(cursorIndex);
			parameterVariable_buttonMessage.set<Guid>("ValueMessageId", cursorGuid);

			Object parameterVariable_leftVisible = selectionPanel.call<Object>("getParameterLegacy(System.String)", str_Bool_LR_Selection_Cursor_Left_Visible);
			parameterVariable_leftVisible.set<bool>("ValueBool", cursorIndex > 0);

			Object parameterVariable_rightVisible = selectionPanel.call<Object>("getParameterLegacy(System.String)", str_Bool_LR_Selection_Cursor_Right_Visible);
			parameterVariable_rightVisible.set<bool>("ValueBool", cursorIndex < cursorMax - 1);
		}
	}
}

/// @brief Updates description message at bottom of screen
/// @param launcherOptionSound app.cLauncherOptionSound object
void AudioMenuExtension::updateGuidMessage(Object launcherOptionSound)
{
	Object guidList = launcherOptionSound["GuidList"];
	std::int32_t selectedIndex = launcherOptionSound.get<std::int32_t>("_SelectedIndex");
	Guid guid = guidList.get<Guid>(selectedIndex);

	launcherOptionSound.call("setGuidMessage", &guid);
}

/// @brief Install all hooks used for audio menu extensions
void AudioMenuExtension::installHooks()
{
	// Hook method which sets up the Audio menu
	// Here we modify the Audio menu to add our new options
	static Object s_launcherOptionSound;
	s_hooks.emplace_back(hook(
		"app.cLauncherOptionSound.setupContent()",
		[](int argc, void **argv, auto...)
		{
			s_launcherOptionSound = Object(argv[1]);

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		[](auto...)
		{
			size_t numNewMenuItems = s_activeMenuItems.size();

			Object launcherOptionSound = s_launcherOptionSound;

			Object nameList = launcherOptionSound["NameList"];
			Object guidList = launcherOptionSound["GuidList"];
			Object cursorIndexList = launcherOptionSound["_CursolIndex"];
			Object cursorMaxList = launcherOptionSound["CursolMax"];
			Object cursorNameListList = launcherOptionSound["_CursolNameList"];

			// If this is a reload, do not update stuff we don't need to
			if (s_newMenuItemsIdx < 0)
			{
				// This is initial load, get original length of list
				s_newMenuItemsIdx = nameList.get<std::int32_t>("Length");

				Object newNameList = createArray("System.Guid", s_newMenuItemsIdx + numNewMenuItems);
				Object newGuidList = createArray("System.Guid", s_newMenuItemsIdx + numNewMenuItems);
				Object newCursorIndexList = createArray("System.Int32", s_newMenuItemsIdx + numNewMenuItems);
				Object newCursorMaxList = createArray("System.Int32", s_newMenuItemsIdx + numNewMenuItems);

				// Fill new arrays
				auto menuItemIter = s_activeMenuItems.begin();
				for (int i = 0; i < s_newMenuItemsIdx + numNewMenuItems; ++i)
				{
					Guid nameGuid;
					Guid descriptionGuid;
					std::int32_t cursorIdx;
					std::int32_t cursorMax;

					if (i < s_newMenuItemsIdx)
					{
						nameGuid = nameList.get<Guid>(i);
						descriptionGuid = guidList.get<Guid>(i);
						cursorIdx = cursorIndexList.get<std::int32_t>(i);
						cursorMax = cursorMaxList.get<std::int32_t>(i);
					}
					else
					{
						// get new item
						std::shared_ptr<MenuItem> menuItem = *menuItemIter;

						nameGuid = menuItem->m_nameGuid;
						descriptionGuid = menuItem->m_descriptionGuid;
						cursorMax = menuItem->m_options->size();
						cursorIdx = menuItem->getCursor();

						++menuItemIter;
					}

					newNameList.set<Guid>(i, nameGuid);
					newGuidList.set<Guid>(i, descriptionGuid);
					newCursorIndexList.set<std::int32_t>(i, cursorIdx);
					newCursorMaxList.set<std::int32_t>(i, cursorMax);
				}

				launcherOptionSound["NameList"] = newNameList;
				launcherOptionSound["GuidList"] = newGuidList;
				launcherOptionSound["_CursolIndex"] = newCursorIndexList;
				launcherOptionSound["CursolMax"] = newCursorMaxList;
			}

			// Cursor name list needs to be rebuilt each time
			{
				Object newCursorNameListList = createArray("System.Array", s_newMenuItemsIdx + numNewMenuItems);

				// Fill new arrays
				auto menuItemIter = s_activeMenuItems.begin();
				for (int i = 0; i < s_newMenuItemsIdx + numNewMenuItems; ++i)
				{
					Object cursorNameList;

					if (i < s_newMenuItemsIdx)
					{
						cursorNameList = cursorNameListList[i];
					}
					else
					{
						// get new item
						std::shared_ptr<MenuItem> menuItem = *menuItemIter;

						// build new cursor name list
						cursorNameList = createArray("System.Guid", menuItem->m_options->size());

						// fill new cursor name list
						for (int j = 0; j < menuItem->m_options->size(); j++)
						{
							// set cursorNameList[j]
							Guid cursorName = (*menuItem->m_options)[j].m_nameGuid;
							cursorNameList.set<Guid>(j, cursorName);
						}

						++menuItemIter;
					}

					newCursorNameListList.set(i, cursorNameList);
				}

				launcherOptionSound.set("_CursolNameList", newCursorNameListList);
			}

			// Show/hide the scrollbar
			Object str_scrollBarPath = createString(L"/FSB_ref_Vertical_Button");
			Object soundList = launcherOptionSound["_SoundList"];

			// set soundList.BarPath
			// This should be done before set_ItemCount
			// BarPath is a reflection property, so use the offset directly...
			reframework::API::ManagedObject **barPath_ptr = (reframework::API::ManagedObject **)((intptr_t)soundList.m_object + FLUENT_SCROLL_LIST_OFFSET_BAR_PATH);
			assert(barPath_ptr != nullptr);
			reframework::API::ManagedObject *barPath = *barPath_ptr;
			if (barPath != nullptr)
			{
				barPath->release();
			}
			*barPath_ptr = str_scrollBarPath.m_object;
			str_scrollBarPath.m_object->add_ref();
			assert(soundList["BarPath"] == str_scrollBarPath); // if this fails then FLUENT_SCROLL_LIST_OFFSET_BAR_PATH has changed

			// set soundList.ItemCount
			soundList.set<std::int32_t>("ItemCount", s_newMenuItemsIdx + numNewMenuItems);

			// call soundList.getObject
			// Technically the game calls getObject(System.String, System.Type),
			// which we could do with tdb->find_type("via.gui.Panel")->get_type(),
			// but we don't care about the strong typing anyway
			Object fluentScrollBar = soundList.call<Object>("getObject(System.String)", str_scrollBarPath);

			if (s_newMenuItemsIdx + numNewMenuItems > ITEMS_PER_PAGE)
			{
				Object str_textPrevPath = createString(L"/txt_Prev");
				Object str_textNextPath = createString(L"/txt_Next");
				assert(str_textPrevPath != nullptr);
				assert(str_textNextPath != nullptr);

				// call fluentScrollBar.getObject
				// Technically the game calls getObject(System.String, System.Type),
				// which we could do with tdb->find_type("via.gui.Panel")->get_type(),
				// but we don't care about the strong typing anyway
				Object textPrev = fluentScrollBar.call<Object>("getObject(System.String)", str_textPrevPath);

				// call fluentScrollBar.getObject
				// Technically the game calls getObject(System.String, System.Type),
				// which we could do with tdb->find_type("via.gui.Panel")->get_type(),
				// but we don't care about the strong typing anyway
				Object textNext = fluentScrollBar.call<Object>("getObject(System.String)", str_textNextPath);

				fluentScrollBar.set<bool>("Visible", true);

				Guid guidTextPrev = L"cbfc4e85-c78c-4632-bd34-0e4a10e41f6b"_guid; // <ICON LAUNCHER_PAGE_PREV>
				textPrev.set<Guid>("MessageId", guidTextPrev);

				Guid guidTextNext = L"14dd8e95-a0c1-462a-acbc-5c2e34498a34"_guid; // <ICON LAUNCHER_PAGE_NEXT>
				textNext.set<Guid>("MessageId", guidTextNext);
			}
			else
			{
				fluentScrollBar.set<bool>("Visible", false);
			}

			updateList(launcherOptionSound);
		}));

	// Hook method which is called every tick while in Audio menu
	// We basically need to replace this whole function with one which can handle list scrolling
	s_hooks.emplace_back(hook(
		"app.cLauncherOptionSound.doUpdate()",
		[](int argc, void **argv, auto...)
		{
			Object launcherOptionSound = Object(argv[1]);

			std::int32_t selectedIndex = launcherOptionSound.get<std::int32_t>("_SelectedIndex");
			Object soundList = launcherOptionSound["_SoundList"];
			Object cursorIndexList = launcherOptionSound["_CursolIndex"];
			std::int32_t cursorIndexListLength = cursorIndexList.get<std::int32_t>("Length");

			// call launcherOptionSound.isUpdateList()
			// returns true if different item is selected and updates selectedIndex
			bool isUpdateList = launcherOptionSound.call<bool>(
				"isUpdateList",
				&selectedIndex, // passed by ref
				cursorIndexListLength);

			if (isUpdateList)
			{
				launcherOptionSound.set<std::int32_t>("_SelectedIndex", selectedIndex);
				soundList.set<std::int32_t>("SelectedIndex", selectedIndex);
				updateList(launcherOptionSound);
				updateGuidMessage(launcherOptionSound);
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			// call launcherOptionSound.isUpdateListPage()
			// returns true if list is page scrolled and updates selectedIndex
			// also calls soundList.set_SelectedIndex internally
			bool isUpdateListPage = launcherOptionSound.call<bool>(
				"isUpdateListPage",
				soundList,
				&selectedIndex // passed by ref
			);

			if (isUpdateListPage)
			{
				launcherOptionSound.set<std::int32_t>("_SelectedIndex", selectedIndex);
				updateList(launcherOptionSound);
				updateGuidMessage(launcherOptionSound);
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			Object cursorMaxList = launcherOptionSound["CursolMax"];
			std::int32_t cursorIndex = cursorIndexList.get<std::int32_t>(selectedIndex);
			std::int32_t cursorMax = cursorMaxList.get<std::int32_t>(selectedIndex);

			// Get menu item
			std::shared_ptr<MenuItem> menuItem = getCustomMenuItem(selectedIndex);

			// call menu.isUpdateSelectedItem()
			// returns true if different value chosen for selected item and updates cursorIndex
			// The game can directly pass &cursorIndexList[selectedIndex], but we can't,
			// so we have to use the temporary variable
			bool isUpdateSelectedItem = launcherOptionSound.call<bool>(
				"isUpdateSelectedItem(System.Int32, System.Int32, System.Boolean)",
				&cursorIndex, // passed by ref
				cursorMax,
				false);

			if (isUpdateSelectedItem)
			{
				cursorIndexList.set<std::int32_t>(selectedIndex, cursorIndex);

				if (menuItem != nullptr)
				{
					std::int32_t value = cursorIndex;
					if (menuItem->m_options != nullptr)
					{
						MenuItem::Option option = (*menuItem->m_options)[cursorIndex];
						value = option.m_value;

						// Show description for this value if it has one
						if (option.m_descriptionGuid.has_value())
						{
							Guid guid = option.m_descriptionGuid.value();
							launcherOptionSound.call("setGuidMessage", &guid);
						}
					}
					menuItem->setValue(value);
				}

				// The game only updates the selected item, but this is probably fine too
				updateList(launcherOptionSound);

				// Propagate settings
				launcherOptionSound.call("updateVolume");

				std::int32_t bgmType = launcherOptionSound.call<std::int32_t>("convertCusrotIndexToBGMType");
				Object launcher = getSingleton("app.Launcher");
				launcher.set<std::int32_t>("bgmType", bgmType);

				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			if (menuItem != nullptr)
			{
				// call app.GameInputManager.isLauncherInputSuccess()
				// returns true if key is pressed on item
				Object gameInputManager = getSingleton("app.GameInputManager");
				bool isInputDecide = gameInputManager.call<bool>("isLauncherInputSuccess", app::LauncherInputKey::KEY::DECIDE);
				if (isInputDecide && menuItem->canEnter() && menuItem->onEnter())
				{
					Object soundMilkyManager = getType("app.sound.SoundMilkyManager")["_Instance"];
					soundMilkyManager.call("playSeById", app::sound::SoundMilkyDefine::SeID::LAUNCHER_OK);

					return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
				}
			}

			return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
		},
		nullptr));

	// Hook method which is called when user exits the Settings menu
	s_hooks.emplace_back(hook(
		"app.GUILauncherOption.onDestroy()",
		[](int argc, void **argv, auto...)
		{
			s_newMenuItemsIdx = -1;

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	// Hook method which resets settings to default
	s_hooks.emplace_back(hook(
		"app.cLauncherOptionSound.resetSetting()",
		[](int argc, void **argv, auto...)
		{
			assert(s_newMenuItemsIdx >= 0);

			Object launcherOptionSound = Object(argv[1]);
			Object cursorIndexList = launcherOptionSound["_CursolIndex"];
			std::int32_t cursorIndexListLength = cursorIndexList.get<std::int32_t>("Length");

			auto menuItemIter = s_activeMenuItems.begin();
			for (std::size_t i = s_newMenuItemsIdx; menuItemIter != s_activeMenuItems.end(); ++i)
			{
				// get new item
				std::shared_ptr<MenuItem> menuItem = *menuItemIter;

				std::int32_t value = menuItem->m_defaultValue;
				menuItem->setValue(value);
				cursorIndexList.set<std::int32_t>(i, value);

				++menuItemIter;
			}

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	s_hooks.emplace_back(hook(
		"app.GUILauncherOperationGuide.setOperationGuide",
		[](int argc, void **argv, auto...)
		{
			Object operationGuide = Object(argv[1]);
			Object indexList = Object(argv[2]);

			Object launcher = getSingleton("app.Launcher");
			// Check if we need to add Edit List to the operation guide
			Object option = launcher.call<Object>("get__OptionComponent");

			// If we are not in options, skip
			if (option == nullptr || s_newMenuItemsIdx < 0)
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			Guid editListGuid = L"079c9622-16d9-4871-b199-c25eb73196c8"_guid;
			std::int32_t editListIdx = -1;

			// Check if Edit List is already added
			Object guideDataList = operationGuide["_OperationGuideData"]["GuideDatas"];
			std::int32_t guideDataListLength = guideDataList.get<std::int32_t>("Length");
			for (editListIdx = 0; editListIdx < guideDataListLength; editListIdx++)
			{
				Guid messageId = guideDataList[editListIdx].get<Guid>("MessageId");
				if (messageId == editListGuid)
				{
					break;
				}
			}

			// Add Edit List to operation guide
			if (editListIdx >= guideDataListLength)
			{
				Object newGuideDataList = createArray("app.OperationGuideData", guideDataListLength + 1);

				for (std::int32_t i = 0; i < guideDataListLength; i++)
				{
					newGuideDataList[i] = guideDataList[i];
				}

				Object editListData = createObject("app.OperationGuideData");
				editListData.call(".ctor");
				editListData.set<Guid>("_MessageId", editListGuid);

				newGuideDataList[editListIdx] = editListData;
				operationGuide["_OperationGuideData"]["_GuideDatas"] = newGuideDataList;
			}

			// If Audio settings is open
			if ((app::GUILauncherOption::MENU_TYPE)option.get<std::int32_t>("_CurrentMenuType") != app::GUILauncherOption::MENU_TYPE::SOUND)
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}
			// If we are inside Audio settings
			if ((app::GUILauncherOption::Routine)option.get<std::int32_t>("_Routine") != app::GUILauncherOption::Routine::UPDATE_MENU)
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			// Check if current menu item is enterable
			std::shared_ptr<MenuItem> menuItem = getCustomMenuItem(option["_CurrentMenu"].get<std::int32_t>("_SelectedIndex"));
			if (menuItem == nullptr || !menuItem->canEnter())
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}

			// Add Edit List to the operation guide
			indexList.call("Add", editListIdx);

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));
}

/// @brief Uninstall all hooks used for audio menu extensions
void AudioMenuExtension::uninstallHooks()
{
	for (HookRef &hook : s_hooks)
	{
		hook.unhook();
	}
	s_hooks.clear();
}