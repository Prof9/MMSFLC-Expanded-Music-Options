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

/// @brief Install all hooks used for audio menu extensions
void AudioMenuExtension::installHooks()
{
	// Hook method which is called when Settings menu is initialized
	// Here we modify the Audio menu to add our new options
	s_hooks.emplace_back(hook(
		"app.GUILauncherOption",
		"start()",
		nullptr,
		[](auto...)
		{
			ptrdiff_t numNewMenuItems = std::ranges::distance(getAllNewMenuItems());

			Object launcher = getSingleton("app.Launcher");
			Object guiBehaviors = launcher.get<Object>("guiBehaviors");
			Object launcherOption = guiBehaviors.get<Object>(std::to_underlying(app::Launcher::LauncherGUIId::Option));
			Object menuTbl = launcherOption.get<Object>("_MenuTbl");

			s_menuTblOption = menuTbl.get<Object>(std::to_underlying(app::GUILauncherOption::MENU_TYPE::SOUND));

			Object nameList = s_menuTblOption.get<Object>("NameList");
			Object guidList = s_menuTblOption.get<Object>("GuidList");
			Object cursorIndexList = s_menuTblOption.get<Object>("_CursolIndex");
			Object cursorMaxList = s_menuTblOption.get<Object>("CursolMax");
			Object cursorNameListList = s_menuTblOption.get<Object>("_CursolNameList");

			s_newSoundOptionsIdx = nameList.get<std::int32_t>("Length");

			Object newNameList = createArray("System.Guid", s_newSoundOptionsIdx + numNewMenuItems);
			Object newGuidList = createArray("System.Guid", s_newSoundOptionsIdx + numNewMenuItems);
			Object newCursorIndexList = createArray("System.Int32", s_newSoundOptionsIdx + numNewMenuItems);
			Object newCursorMaxList = createArray("System.Int32", s_newSoundOptionsIdx + numNewMenuItems);
			Object newCursorNameListList = createArray("System.Array", s_newSoundOptionsIdx + numNewMenuItems);

			// Fill new arrays
			auto menuItemGen = getAllNewMenuItems();
			auto menuItemIter = menuItemGen.begin();
			for (int i = 0; i < s_newSoundOptionsIdx + numNewMenuItems; ++i)
			{
				Guid nameGuid;
				Guid descriptionGuid;
				std::int32_t cursorIdx;
				std::int32_t cursorMax;
				Object cursorNameList;

				if (i < s_newSoundOptionsIdx)
				{
					nameGuid = nameList.get<Guid>(i);
					descriptionGuid = guidList.get<Guid>(i);
					cursorIdx = cursorIndexList.get<std::int32_t>(i);
					cursorMax = cursorMaxList.get<std::int32_t>(i);
					cursorNameList = cursorNameListList.get<Object>(i);
				}
				else
				{
					// get new item
					MenuItem const *menuItem = *menuItemIter;

					nameGuid = menuItem->NameGuid;
					descriptionGuid = menuItem->DescriptionGuid;
					cursorMax = menuItem->SelectionOptions->size();
					cursorIdx = *menuItem->SettingPtr;

					// If setting value is out of bounds, reset to default
					if (cursorIdx < 0 || cursorIdx >= cursorMax)
					{
						*menuItem->SettingPtr = cursorIdx = 0;
					}

					// build new cursor name list
					// cursorNameList = new System.Array<System.Guid>[cursorMax]
					cursorNameList = createArray("System.Guid", cursorMax);

					// fill new cursor name list
					for (int j = 0; j < cursorMax; j++)
					{
						// set cursorNameList[j]
						Guid cursorName = (*menuItem->SelectionOptions)[j];
						cursorNameList.set<Guid>(j, cursorName);
					}

					++menuItemIter;
				}

				newNameList.set<Guid>(i, nameGuid);
				newGuidList.set<Guid>(i, descriptionGuid);
				newCursorIndexList.set<std::int32_t>(i, cursorIdx);
				newCursorMaxList.set<std::int32_t>(i, cursorMax);
				newCursorNameListList.set<Object>(i, cursorNameList);
			}

			s_menuTblOption.set<Object>("NameList", newNameList);
			s_menuTblOption.set<Object>("GuidList", newGuidList);
			s_menuTblOption.set<Object>("_CursolIndex", newCursorIndexList);
			s_menuTblOption.set<Object>("CursolMax", newCursorMaxList);
			s_menuTblOption.set<Object>("_CursolNameList", newCursorNameListList);

			// Show/hide the scrollbar
			Object str_scrollBarPath = createString(L"/FSB_ref_Vertical_Button");
			Object soundList = s_menuTblOption.get<Object>("_SoundList");

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
		}));

	// Hook method which sets up the Audio menu
	s_doResizeFluentScrollList = false;
	s_hooks.emplace_back(hook(
		"app.cLauncherOptionSound",
		"setupContent()",
		[](int argc, void **argv, auto...)
		{
			s_menuTblOption = Object(argv[1]);

			s_doResizeFluentScrollList = true;

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	// Hook first TDB method which gets called after _SoundList is initialized in setupContent()
	// Here we change the number of items in the list
	s_hooks.emplace_back(hook(
		"via.gui.FluentScrollList",
		"get_ItemInstTbl",
		[](int argc, void **argv, auto...)
		{
			if (!s_doResizeFluentScrollList)
			{
				return REFRAMEWORK_HOOK_CALL_ORIGINAL;
			}
			s_doResizeFluentScrollList = false;

			ptrdiff_t numNewMenuItems = std::ranges::distance(getAllNewMenuItems());

			Object soundList = Object(argv[1]);

			Object str_scrollBarPath = createString(L"/FSB_ref_Vertical_Button");

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

			s_newSoundOptionsIdx = soundList.get<int32_t>("ItemCount");

			// set soundList.ItemCount
			soundList.set<std::int32_t>("ItemCount", s_newSoundOptionsIdx + numNewMenuItems);

			return REFRAMEWORK_HOOK_CALL_ORIGINAL;
		},
		nullptr));

	// Hook method which is called every tick while in Audio menu
	// We basically need to replace this whole function with one which can handle list scrolling
	s_hooks.emplace_back(hook(
		"app.cLauncherOptionSound",
		"doUpdate()",
		[](int argc, void **argv, auto...)
		{
			auto updateList = [](Object menu)
			{
				Object soundList = menu.get<Object>("_SoundList");

				Object itemInstTbl = soundList.get<Object>("ItemInstTbl");
				std::int32_t itemInstTblLength = itemInstTbl.get<std::int32_t>("Length");

				// get menu.NameList
				Object nameList = menu.get<Object>("NameList");
				Object cursorIndexList = menu.get<Object>("_CursolIndex");
				Object cursorMaxList = menu.get<Object>("CursolMax");
				Object cursorNameListList = menu.get<Object>("_CursolNameList");
				Object volumeInitValueList = menu.get<Object>("VolumeInitValue");

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
			};
			auto updateGuidMessage = [](Object menu)
			{
				Object guidList = menu.get<Object>("GuidList");
				std::int32_t selectedIndex = menu.get<std::int32_t>("_SelectedIndex");
				Guid guid = guidList.get<Guid>(selectedIndex);

				menu.call<void>("setGuidMessage(System.Guid)", {&guid});
			};

			ptrdiff_t numNewMenuItems = std::ranges::distance(getAllNewMenuItems());

			Object menu = Object(argv[1]);

			std::int32_t selectedIndex = menu.get<std::int32_t>("_SelectedIndex");
			Object soundList = menu.get<Object>("_SoundList");

			// call menu.isUpdateList()
			// returns true if different item is selected and updates selectedIndex
			bool isUpdateList = menu.call<bool>(
				"isUpdateList",
				{
					&selectedIndex, // passed by ref
					(void *)(intptr_t)(s_newSoundOptionsIdx + numNewMenuItems),
				});

			if (isUpdateList)
			{
				menu.set<std::int32_t>("_SelectedIndex", selectedIndex);
				soundList.set<std::int32_t>("SelectedIndex", selectedIndex);
				updateList(menu);
				updateGuidMessage(menu);
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			// call menu.isUpdateListPage()
			// returns true if list is page scrolled and updates selectedIndex
			// also calls soundList.set_SelectedIndex internally
			bool isUpdateListPage = menu.call<bool>(
				"isUpdateListPage",
				{
					soundList,
					&selectedIndex, // passed by ref
				});

			if (isUpdateListPage)
			{
				menu.set<std::int32_t>("_SelectedIndex", selectedIndex);
				updateList(menu);
				updateGuidMessage(menu);
				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			Object cursorIndexList = menu.get<Object>("_CursolIndex");
			Object cursorMaxList = menu.get<Object>("CursolMax");
			std::int32_t cursorIndex = cursorIndexList.get<std::int32_t>(selectedIndex);
			std::int32_t cursorMax = cursorMaxList.get<std::int32_t>(selectedIndex);

			// call menu.isUpdateSelectedItem()
			// returns true if different value chosen for selected item and updates cursorIndex
			// The game can directly pass &cursorIndexList[selectedIndex], but we can't,
			// so we have to use the temporary variable
			bool isUpdateSelectedItem = menu.call<bool>(
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
				updateList(menu);

				// Propagate settings
				menu.call<void>("updateVolume", {});

				std::int32_t bgmType = menu.call<std::int32_t>("convertCusrotIndexToBGMType", {});
				Object launcher = getSingleton("app.Launcher");
				launcher.set<std::int32_t>("bgmType", bgmType);

				return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
			}

			return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
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