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
	Object soundList = launcherOptionSound.get<Object>("_SoundList");
	Object nameList = launcherOptionSound.get<Object>("NameList");
	Object cursorIndexList = launcherOptionSound.get<Object>("_CursolIndex");
	Object cursorMaxList = launcherOptionSound.get<Object>("CursolMax");
	Object cursorNameListList = launcherOptionSound.get<Object>("_CursolNameList");
	Object volumeInitValueList = launcherOptionSound.get<Object>("VolumeInitValue");

	Object itemInstTbl = soundList.get<Object>("ItemInstTbl");
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
		Object selectItem = itemInstTbl.get<Object>(i);
		std::int32_t listIndex = selectItem.get<std::int32_t>("ListIndex");

		// Normally the game will compute a MurmurHash3 of the string, but this is easier
		Object parameterVariable = selectItem.call<Object>("getParameterLegacy(System.String)", {str_MsgId_Option_List_Item_Name});
		Guid nameGuid = nameList.get<Guid>(listIndex);
		parameterVariable.set<Guid>("ValueMessageId", {nameGuid});

		Object cursorNameList = cursorNameListList.get<Object>(listIndex);
		std::int32_t cursorIndex = cursorIndexList.get<std::int32_t>(listIndex);
		std::int32_t cursorMax = cursorMaxList.get<std::int32_t>(listIndex);

		// assume option is a slider type if it has no name list
		if (cursorNameList.null())
		{
			selectItem.set<std::uint32_t>("StatePattern", patternSlider);

			parameterVariable = selectItem.call<Object>("getParameterLegacy(System.String)", {str_ObjPath_PNL_Option_List_Item_Slider_Path});
			Object objPath = parameterVariable.get<Object>("ValueObjectPath");
			Object sliderPanel = selectItem.call<Object>("getObject(System.String)", {objPath});

			Object paramVariable_floatSliderValueInit = sliderPanel.call<Object>("getParameterLegacy(System.String)", {str_Float_Slider_Value_Init});
			double volumeInitValue = volumeInitValueList.get<double>(listIndex);
			double sliderValueInit = volumeInitValue / (cursorMax - 1) * 100.0;
			paramVariable_floatSliderValueInit.set<double>("ValueFloat", sliderValueInit);

			Object paramVariable_floatSliderValueCurrent = sliderPanel.call<Object>("getParameterLegacy(System.String)", {str_Float_Slider_Value_Current});
			double sliderValueCurrent = (double)cursorIndex / (cursorMax - 1) * 100.0;
			paramVariable_floatSliderValueCurrent.set<double>("ValueFloat", sliderValueCurrent);
		}
		else
		{
			selectItem.set<std::uint32_t>("StatePattern", patternLRSelection);

			parameterVariable = selectItem.call<Object>("getParameterLegacy(System.String)", {str_ObjPath_PNL_Option_List_Item_LR_Selection_Path});
			Object objPath = parameterVariable.get<Object>("ValueObjectPath");
			Object selectionPanel = selectItem.call<Object>("getObject(System.String)", {objPath});

			Object parameterVariable_buttonMessage = selectionPanel.call<Object>("getParameterLegacy(System.String)", {str_MsgId_LR_Selection_Button_Message});
			Guid cursorGuid = cursorNameList.get<Guid>(cursorIndex);
			parameterVariable_buttonMessage.set<Guid>("ValueMessageId", cursorGuid);

			Object parameterVariable_leftVisible = selectionPanel.call<Object>("getParameterLegacy(System.String)", {str_Bool_LR_Selection_Cursor_Left_Visible});
			parameterVariable_leftVisible.set<bool>("ValueBool", cursorIndex > 0);

			Object parameterVariable_rightVisible = selectionPanel.call<Object>("getParameterLegacy(System.String)", {str_Bool_LR_Selection_Cursor_Right_Visible});
			parameterVariable_rightVisible.set<bool>("ValueBool", cursorIndex < cursorMax - 1);
		}
	}
}

/// @brief Updates description message at bottom of screen
/// @param launcherOptionSound app.cLauncherOptionSound object
void AudioMenuExtension::updateGuidMessage(Object launcherOptionSound)
{
	Object guidList = launcherOptionSound.get<Object>("GuidList");
	std::int32_t selectedIndex = launcherOptionSound.get<std::int32_t>("_SelectedIndex");
	Guid guid = guidList.get<Guid>(selectedIndex);

	launcherOptionSound.call<void>("setGuidMessage(System.Guid)", {&guid});
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
			ptrdiff_t numNewMenuItems = std::ranges::distance(getAllNewMenuItems());

			Object launcherOptionSound = s_launcherOptionSound;

			Object nameList = launcherOptionSound.get<Object>("NameList");
			Object guidList = launcherOptionSound.get<Object>("GuidList");
			Object cursorIndexList = launcherOptionSound.get<Object>("_CursolIndex");
			Object cursorMaxList = launcherOptionSound.get<Object>("CursolMax");
			Object cursorNameListList = launcherOptionSound.get<Object>("_CursolNameList");

			// If this is a reload, do not update stuff we don't need to
			if (s_newSoundOptionsIdx < 0)
			{
				// This is initial load, get original length of list
				s_newSoundOptionsIdx = nameList.get<std::int32_t>("Length");

				Object newNameList = createArray("System.Guid", s_newSoundOptionsIdx + numNewMenuItems);
				Object newGuidList = createArray("System.Guid", s_newSoundOptionsIdx + numNewMenuItems);
				Object newCursorIndexList = createArray("System.Int32", s_newSoundOptionsIdx + numNewMenuItems);
				Object newCursorMaxList = createArray("System.Int32", s_newSoundOptionsIdx + numNewMenuItems);

				// Fill new arrays
				auto menuItemGen = getAllNewMenuItems();
				auto menuItemIter = menuItemGen.begin();
				for (int i = 0; i < s_newSoundOptionsIdx + numNewMenuItems; ++i)
				{
					Guid nameGuid;
					Guid descriptionGuid;
					std::int32_t cursorIdx;
					std::int32_t cursorMax;

					if (i < s_newSoundOptionsIdx)
					{
						nameGuid = nameList.get<Guid>(i);
						descriptionGuid = guidList.get<Guid>(i);
						cursorIdx = cursorIndexList.get<std::int32_t>(i);
						cursorMax = cursorMaxList.get<std::int32_t>(i);
					}
					else
					{
						// get new item
						MenuItem &menuItem = *menuItemIter;

						nameGuid = menuItem.m_nameGuid;
						descriptionGuid = menuItem.m_descriptionGuid;
						cursorMax = menuItem.m_valueNames->size();
						cursorIdx = menuItem.getValue();

						++menuItemIter;
					}

					newNameList.set<Guid>(i, nameGuid);
					newGuidList.set<Guid>(i, descriptionGuid);
					newCursorIndexList.set<std::int32_t>(i, cursorIdx);
					newCursorMaxList.set<std::int32_t>(i, cursorMax);
				}

				launcherOptionSound.set<Object>("NameList", newNameList);
				launcherOptionSound.set<Object>("GuidList", newGuidList);
				launcherOptionSound.set<Object>("_CursolIndex", newCursorIndexList);
				launcherOptionSound.set<Object>("CursolMax", newCursorMaxList);
			}

			// Cursor name list needs to be rebuilt each time
			{
				Object newCursorNameListList = createArray("System.Array", s_newSoundOptionsIdx + numNewMenuItems);

				// Fill new arrays
				auto menuItemGen = getAllNewMenuItems();
				auto menuItemIter = menuItemGen.begin();
				for (int i = 0; i < s_newSoundOptionsIdx + numNewMenuItems; ++i)
				{
					Object cursorNameList;

					if (i < s_newSoundOptionsIdx)
					{
						cursorNameList = cursorNameListList.get<Object>(i);
					}
					else
					{
						// get new item
						MenuItem &menuItem = *menuItemIter;

						// build new cursor name list
						cursorNameList = createArray("System.Guid", menuItem.m_valueNames->size());

						// fill new cursor name list
						for (int j = 0; j < menuItem.m_valueNames->size(); j++)
						{
							// set cursorNameList[j]
							Guid cursorName = (*menuItem.m_valueNames)[j];
							cursorNameList.set<Guid>(j, cursorName);
						}

						++menuItemIter;
					}

					newCursorNameListList.set<Object>(i, cursorNameList);
				}

				launcherOptionSound.set<Object>("_CursolNameList", newCursorNameListList);
			}

			// Show/hide the scrollbar
			Object str_scrollBarPath = createString(L"/FSB_ref_Vertical_Button");
			Object soundList = launcherOptionSound.get<Object>("_SoundList");

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

			// set soundList.ItemCount
			soundList.set<std::int32_t>("ItemCount", s_newSoundOptionsIdx + numNewMenuItems);

			// call soundList.getObject
			// Technically the game calls getObject(System.String, System.Type),
			// which we could do with tdb->find_type("via.gui.Panel")->get_type(),
			// but we don't care about the strong typing anyway
			Object fluentScrollBar = soundList.call<Object>("getObject(System.String)", {str_scrollBarPath});

			if (s_newSoundOptionsIdx + numNewMenuItems > ITEMS_PER_PAGE)
			{
				Object str_textPrevPath = createString(L"/txt_Prev");
				Object str_textNextPath = createString(L"/txt_Next");
				assert(str_textPrevPath != nullptr);
				assert(str_textNextPath != nullptr);

				// call fluentScrollBar.getObject
				// Technically the game calls getObject(System.String, System.Type),
				// which we could do with tdb->find_type("via.gui.Panel")->get_type(),
				// but we don't care about the strong typing anyway
				Object textPrev = fluentScrollBar.call<Object>("getObject(System.String)", {str_textPrevPath});

				// call fluentScrollBar.getObject
				// Technically the game calls getObject(System.String, System.Type),
				// which we could do with tdb->find_type("via.gui.Panel")->get_type(),
				// but we don't care about the strong typing anyway
				Object textNext = fluentScrollBar.call<Object>("getObject(System.String)", {str_textNextPath});

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
			Object soundList = launcherOptionSound.get<Object>("_SoundList");
			Object cursorIndexList = launcherOptionSound.get<Object>("_CursolIndex");
			std::int32_t cursorIndexListLength = cursorIndexList.get<std::int32_t>("Length");

			// call launcherOptionSound.isUpdateList()
			// returns true if different item is selected and updates selectedIndex
			bool isUpdateList = launcherOptionSound.call<bool>(
				"isUpdateList",
				{
					&selectedIndex, // passed by ref
					(void *)(intptr_t)(cursorIndexListLength),
				});

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
				{
					soundList,
					&selectedIndex, // passed by ref
				});

			if (isUpdateListPage)
			{
				launcherOptionSound.set<std::int32_t>("_SelectedIndex", selectedIndex);
				updateList(launcherOptionSound);
				updateGuidMessage(launcherOptionSound);
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			Object cursorMaxList = launcherOptionSound.get<Object>("CursolMax");
			std::int32_t cursorIndex = cursorIndexList.get<std::int32_t>(selectedIndex);
			std::int32_t cursorMax = cursorMaxList.get<std::int32_t>(selectedIndex);

			// call menu.isUpdateSelectedItem()
			// returns true if different value chosen for selected item and updates cursorIndex
			// The game can directly pass &cursorIndexList[selectedIndex], but we can't,
			// so we have to use the temporary variable
			bool isUpdateSelectedItem = launcherOptionSound.call<bool>(
				"isUpdateSelectedItem(System.Int32, System.Int32, System.Boolean)",
				{
					&cursorIndex, // passed by ref
					(void *)(intptr_t)cursorMax,
					(void *)(intptr_t)false,
				});

			if (isUpdateSelectedItem)
			{
				cursorIndexList.set<std::int32_t>(selectedIndex, cursorIndex);

				// The game only updates the selected item, but this is probably fine too
				updateList(launcherOptionSound);

				// Propagate settings
				launcherOptionSound.call<void>("updateVolume");

				std::int32_t bgmType = launcherOptionSound.call<std::int32_t>("convertCusrotIndexToBGMType");
				Object launcher = getSingleton("app.Launcher");
				launcher.set<std::int32_t>("bgmType", bgmType);

				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			if (selectedIndex >= s_newSoundOptionsIdx)
			{
				// Get menu item
				auto menuItemGen = getAllNewMenuItems();
				auto menuItemIter = menuItemGen.begin();
				std::ranges::advance(menuItemIter, selectedIndex - s_newSoundOptionsIdx);
				MenuItem &menuItem = *menuItemIter;

				// call app.GameInputManager.isLauncherInputSuccess()
				// returns true if key is pressed on item
				Object gameInputManager = getSingleton("app.GameInputManager");
				bool isInputDecide = gameInputManager.call<bool>(
					"isLauncherInputSuccess(app.LauncherInputKey.KEY)",
					{
						(void *)(intptr_t)app::LauncherInputKey::KEY::DECIDE,
					});
				if (isInputDecide && menuItem.onEnter())
				{
					Object soundMilkyManager = getType("app.sound.SoundMilkyManager").get<Object>("_Instance");
					soundMilkyManager.call<void>(
						"playSeById(System.UInt16)",
						{
							(void *)(intptr_t)app::sound::SoundMilkyDefine::SeID::LAUNCHER_OK,
						});

					return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
				}
			}

			return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
		},
		nullptr));

	// Hook method which is called when user exits the Settings menu
	// Here we retrieve the new setting values from the menu elements
	s_hooks.emplace_back(hook(
		"app.GUILauncherOption.onDestroy()",
		[](int argc, void **argv, auto...)
		{
			assert(s_newSoundOptionsIdx >= 0);

			Object guiLauncherOption = Object(argv[1]);

			Object menuTbl = guiLauncherOption.get<Object>("_MenuTbl");
			Object launcherOptionSound = menuTbl.get<Object>(std::to_underlying(app::GUILauncherOption::MENU_TYPE::SOUND));
			Object cursorIndexList = launcherOptionSound.get<Object>("_CursolIndex");
			std::int32_t cursorIndexListLength = cursorIndexList.get<std::int32_t>("Length");

			auto menuItemGen = getAllNewMenuItems();
			auto menuItemIter = menuItemGen.begin();
			for (std::size_t i = s_newSoundOptionsIdx; i < cursorIndexListLength; i++)
			{
				// get new item
				MenuItem &menuItem = *menuItemIter;

				menuItem.setValue(cursorIndexList.get<std::int32_t>(i));

				++menuItemIter;
			}

			s_newSoundOptionsIdx = -1;

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	// Hook method which resets settings to default
	s_hooks.emplace_back(hook(
		"app.cLauncherOptionSound.resetSetting()",
		[](int argc, void **argv, auto...)
		{
			assert(s_newSoundOptionsIdx >= 0);

			Object launcherOptionSound = Object(argv[1]);
			Object cursorIndexList = launcherOptionSound.get<Object>("_CursolIndex");
			std::int32_t cursorIndexListLength = cursorIndexList.get<std::int32_t>("Length");

			auto menuItemGen = getAllNewMenuItems();
			auto menuItemIter = menuItemGen.begin();
			for (std::size_t i = s_newSoundOptionsIdx; i < cursorIndexListLength; i++)
			{
				// get new item
				MenuItem &menuItem = *menuItemIter;

				std::int32_t value = menuItem.m_defaultValue;
				menuItem.setValue(value);
				cursorIndexList.set<std::int32_t>(i, value);

				++menuItemIter;
			}

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