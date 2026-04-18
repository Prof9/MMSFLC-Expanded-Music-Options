#include <Enums_Internal.hpp>

#include <cassert>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdfloat>
#include <thread>
#include <utility>
#include <vector>

#include <Windows.h>

#include <reframework/API.hpp>

#include "Guid.hpp"
#include "MessageManager.hpp"
#include "MessageUtility.hpp"

#define CONFIG_DIR "reframework\\data\\ExpandedMusicOptions"
#define CONFIG_FILENAME CONFIG_DIR "\\config.bin"

struct MenuItem
{
public:
    Guid NameGuid;
    Guid DescriptionGuid;
    uint8_t *SettingPtr;
};

static std::uint8_t _bgmFieldSetting = 0;
static std::uint8_t _bgmBattleSetting = 0;

static bool _doResizeSoundList = false;
static bool _isUpdateListPage = false;

static int _newSoundOptionsIdx = -1;

static const std::vector<MenuItem> NEW_MENU_ITEMS = {
    MenuItem{
        .NameGuid = Guid(L"9dd4d43c-4b86-48f5-9b6f-d1816656760f"),
        .DescriptionGuid = Guid(L"45cf3ad4-b931-4885-a4fe-8e26be1b1475"),
        .SettingPtr = &_bgmFieldSetting,
    },
    MenuItem{
        .NameGuid = Guid(L"aeed2fdf-2495-4fe7-9b9d-afdd92a1a631"),
        .DescriptionGuid = Guid(L"295acd67-b442-4328-af81-505199b3194a"),
        .SettingPtr = &_bgmBattleSetting,
    },
};

static const std::vector<Guid> NEW_BGM_SELECTION_OPTIONS = {
    Guid(L"225f5de3-b958-4c53-b33c-3d2ff2bad06f"), // Original
    Guid(L"37ea887b-9d75-42b2-bd0b-2806cb6d688a"), // Arranged
    Guid(L"09d503da-bb7a-49c7-b920-844b91b4cba1"), // Favorites
    Guid(L"0f2bff24-8e0d-4e6d-851f-0ec36763e53e"), // Off
};

static reframework::API::ManagedObject *_battlePlayer = nullptr;
static reframework::API::ManagedObject *_menuTblOption = nullptr;

static void soundUpdateList(void)
{
}

extern "C" __declspec(dllexport) void reframework_plugin_required_version(REFrameworkPluginVersion *version)
{
    version->major = REFRAMEWORK_PLUGIN_VERSION_MAJOR;
    version->minor = REFRAMEWORK_PLUGIN_VERSION_MINOR;
    version->patch = REFRAMEWORK_PLUGIN_VERSION_PATCH;
}

extern "C" __declspec(dllexport) bool reframework_plugin_initialize(const REFrameworkPluginInitializeParam *param)
{
    auto &api = reframework::API::initialize(param);
    auto tdb = api->tdb();

    // Load settings from file
    std::ifstream configFile;
    configFile.open(CONFIG_FILENAME, std::fstream::in | std::fstream::binary);
    if (configFile.is_open())
    {
        try
        {
            configFile.read((char *)&_bgmFieldSetting, sizeof(_bgmFieldSetting));
            configFile.read((char *)&_bgmBattleSetting, sizeof(_bgmFieldSetting));
        }
        catch (...)
        {
            api->log_error("Failed to load settings from file!");
        }
        configFile.close();
    }

    // Wait for Launcher to initialize
    reframework::API::ManagedObject *launcher = nullptr;
    for (int r = 0; r < 100; ++r)
    {
        launcher = api->get_managed_singleton("app.Launcher");
        if (launcher != nullptr)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    assert(launcher != nullptr);

    // Get MessageManager
    void *messageManagerNativeObject = api->get_native_singleton("via.gui.MessageManager");
    assert(messageManagerNativeObject != nullptr);
    MessageManager messageManager(messageManagerNativeObject);

    /*
    // Hook method which is called when exiting options tab for sound
    // Here we want to avoid writing the settings to the save file,
    // because doing so causes the game to freeze if it tries to load the
    // options menu without this mod active.
    // We also write the modified settings  to file here, which is a little
    // earlier than the game does it, but w/e
    // (Game does it when you exit Settings altogether)
    reframework::API::Method *removeMenuMethod = tdb->find_method("app.cLauncherOptionSound", "removeMenu()");
    assert(removeMenuMethod != nullptr);
    removeMenuMethod->add_hook(
        [](int argc, void **argv, auto...)
        {
            _menuTblOption = (reframework::API::ManagedObject *)argv[1];

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        [](auto...)
        {
            reframework::InvokeRet x;
            auto &api = reframework::API::get();
            auto tdb = api->tdb();

            assert(_menuTblOption != nullptr);
            assert(_newSoundOptionsIdx != -1);

            reframework::API::ManagedObject *launcher = api->get_managed_singleton("app.Launcher");
            assert(launcher != nullptr);

            // set launcher.bgmType to default
            std::int32_t *bgmType_ptr = launcher->get_field<std::int32_t>("bgmType");
            assert(bgmType_ptr != nullptr);
            *bgmType_ptr = 0;

            // get launcherOptionDifficult._CursolIndex
            reframework::API::ManagedObject **cursorIndex_ptr = _menuTblOption->get_field<reframework::API::ManagedObject *>("_CursolIndex");
            assert(cursorIndex_ptr != nullptr);
            reframework::API::ManagedObject *cursorIndex = *cursorIndex_ptr;
            assert(cursorIndex != nullptr);

            // get _CursolIndex[damageReductionIdx]
            /*
            x = cursorIndex->invoke(
                "get_Item",
                {
                    (void *)(intptr_t)_damageReductionIdx,
                });
            int damageMultiplierIdx = x.dword;

            // set damage multiplier
            _damageMultiplier = NEW_DAMAGE_DOWN_VALUES[damageMultiplierIdx].second;
            */
    /*

  // write settings to file
  std::filesystem::create_directories(CONFIG_DIR);
  std::ofstream configFile;
  configFile.open(CONFIG_FILENAME, std::fstream::out | std::fstream::binary);
  if (configFile.is_open())
  {
      configFile.write((char *)&_bgmFieldSetting, sizeof(_bgmFieldSetting));
      configFile.write((char *)&_bgmBattleSetting, sizeof(_bgmBattleSetting));
      configFile.close();
  }
  else
  {
      api->log_error("Failed to save settings to file!");
  }
},
false);
*/

    // Hook method which is called when Settings menu is initialized
    // Here we modify the Audio menu to add our new options
    reframework::API::Method *startMethod = tdb->find_method("app.GUILauncherOption", "start()");
    assert(startMethod != nullptr);
    startMethod->add_hook(
        nullptr,
        [](auto...)
        {
            reframework::InvokeRet x;
            auto &api = reframework::API::get();
            auto tdb = api->tdb();

            reframework::API::ManagedObject *launcher = api->get_managed_singleton("app.Launcher");
            assert(launcher != nullptr);

            // get launcher.guiBehaviors
            reframework::API::ManagedObject **guiBehaviors_ptr = launcher->get_field<reframework::API::ManagedObject *>("guiBehaviors");
            assert(guiBehaviors_ptr != nullptr);
            reframework::API::ManagedObject *guiBehaviors = *guiBehaviors_ptr;
            assert(guiBehaviors != nullptr);

            // get guiBehaviors[app::Launcher::LauncherGUIId::Option]
            x = guiBehaviors->invoke(
                "get_Item",
                {
                    (void *)(intptr_t)std::to_underlying(app::Launcher::LauncherGUIId::Option),
                });
            reframework::API::ManagedObject *launcherOption = (reframework::API::ManagedObject *)x.ptr;
            assert(launcherOption != nullptr);

            // get launcherOption._MenuTbl
            reframework::API::ManagedObject **menuTbl_ptr = launcherOption->get_field<reframework::API::ManagedObject *>("_MenuTbl");
            assert(menuTbl_ptr != nullptr);
            reframework::API::ManagedObject *menuTbl = *menuTbl_ptr;
            assert(menuTbl != nullptr);

            // get menuTbl[app::GUILauncherOption::MENU_TYPE::SOUND]
            x = menuTbl->invoke(
                "get_Item",
                {
                    (void *)(intptr_t)std::to_underlying(app::GUILauncherOption::MENU_TYPE::SOUND),
                });
            _menuTblOption = (reframework::API::ManagedObject *)x.ptr;
            assert(_menuTblOption != nullptr);

            // get menuTblOption.NameList
            reframework::API::ManagedObject **nameList_ptr = _menuTblOption->get_field<reframework::API::ManagedObject *>("NameList");
            assert(nameList_ptr != nullptr);
            reframework::API::ManagedObject *nameList = *nameList_ptr;
            assert(nameList != nullptr);

            // get menuTblOption.GuidList (description list)
            reframework::API::ManagedObject **guidList_ptr = _menuTblOption->get_field<reframework::API::ManagedObject *>("GuidList");
            assert(guidList_ptr != nullptr);
            reframework::API::ManagedObject *guidList = *guidList_ptr;
            assert(guidList != nullptr);

            // get menuTblOption._CursolIndex
            reframework::API::ManagedObject **cursorIndexList_ptr = _menuTblOption->get_field<reframework::API::ManagedObject *>("_CursolIndex");
            assert(cursorIndexList_ptr != nullptr);
            reframework::API::ManagedObject *cursorIndexList = *cursorIndexList_ptr;
            assert(cursorIndexList != nullptr);

            // get menuTblOption.CursolMax
            reframework::API::ManagedObject **cursorMaxList_ptr = _menuTblOption->get_field<reframework::API::ManagedObject *>("CursolMax");
            assert(cursorMaxList_ptr != nullptr);
            reframework::API::ManagedObject *cursorMaxList = *cursorMaxList_ptr;
            assert(cursorMaxList != nullptr);

            // get menuTblOption._CursolNameList
            reframework::API::ManagedObject **cursorNameListList_ptr = _menuTblOption->get_field<reframework::API::ManagedObject *>("_CursolNameList");
            assert(cursorNameListList_ptr != nullptr);
            reframework::API::ManagedObject *cursorNameListList = *cursorNameListList_ptr;
            assert(cursorNameListList != nullptr);

            // get nameList.Length
            x = nameList->invoke(
                "get_Length",
                {});
            _newSoundOptionsIdx = (int)x.dword;

            // get System.Guid type
            reframework::API::TypeDefinition *guidType = tdb->find_type("System.Guid");
            assert(guidType != nullptr);

            // get System.Int32 type
            reframework::API::TypeDefinition *int32Type = tdb->find_type("System.Int32");
            assert(int32Type != nullptr);

            // get System.Array type
            reframework::API::TypeDefinition *arrayType = tdb->find_type("System.Array");
            assert(arrayType != nullptr);

            // newNameList = new System.Guid[_newSoundOptionsIdx + NEW_MENU_ITEMS.size()]
            reframework::API::ManagedObject *newNameList = api->create_managed_array(guidType, _newSoundOptionsIdx + NEW_MENU_ITEMS.size());
            assert(newNameList != nullptr);
            newNameList->add_ref();

            // newGuidList = new System.Guid[_newSoundOptionsIdx + NEW_MENU_ITEMS.size()]
            reframework::API::ManagedObject *newGuidList = api->create_managed_array(guidType, _newSoundOptionsIdx + NEW_MENU_ITEMS.size());
            assert(newGuidList != nullptr);
            newGuidList->add_ref();

            // newCursorIndexList = new System.Int32[_newSoundOptionsIdx + NEW_MENU_ITEMS.size()]
            reframework::API::ManagedObject *newCursorIndexList = api->create_managed_array(int32Type, _newSoundOptionsIdx + NEW_MENU_ITEMS.size());
            assert(newCursorIndexList != nullptr);
            newCursorIndexList->add_ref();

            // newCursorMaxList = new System.Int32[_newSoundOptionsIdx + NEW_MENU_ITEMS.size()]
            reframework::API::ManagedObject *newCursorMaxList = api->create_managed_array(int32Type, _newSoundOptionsIdx + NEW_MENU_ITEMS.size());
            assert(newCursorMaxList != nullptr);
            newCursorMaxList->add_ref();

            // newCursorNameListList = new System.Array[_newSoundOptionsIdx + NEW_MENU_ITEMS.size()]
            reframework::API::ManagedObject *newCursorNameListList = api->create_managed_array(arrayType, _newSoundOptionsIdx + NEW_MENU_ITEMS.size());
            assert(newCursorNameListList != nullptr);
            newCursorNameListList->add_ref();

            // Fill new arrays
            for (int i = 0; i < _newSoundOptionsIdx + NEW_MENU_ITEMS.size(); ++i)
            {
                Guid nameGuid;
                Guid descriptionGuid;
                std::int32_t cursorIdx;
                std::int32_t cursorMax;
                reframework::API::ManagedObject *cursorNameList = nullptr;

                if (i < _newSoundOptionsIdx)
                {
                    // get nameList[i]
                    x = nameList->invoke(
                        "get_Item",
                        {
                            (void *)(intptr_t)i,
                        });
                    nameGuid = *(Guid *)&x;

                    // get guidList[i]
                    x = guidList->invoke(
                        "get_Item",
                        {
                            (void *)(intptr_t)i,
                        });
                    descriptionGuid = *(Guid *)&x;

                    // get cursorIndexList[i]
                    x = cursorIndexList->invoke(
                        "get_Item",
                        {
                            (void *)(intptr_t)i,
                        });
                    cursorIdx = (int32_t)x.dword;

                    // get cursorMaxList[i]
                    x = cursorMaxList->invoke(
                        "get_Item",
                        {
                            (void *)(intptr_t)i,
                        });
                    cursorMax = (int32_t)x.dword;

                    // get cursorNameListList[i]
                    x = cursorNameListList->invoke(
                        "get_Item",
                        {
                            (void *)(intptr_t)i,
                        });
                    cursorNameList = (reframework::API::ManagedObject *)x.ptr;
                }
                else
                {
                    // get new item
                    nameGuid = NEW_MENU_ITEMS[i - _newSoundOptionsIdx].NameGuid;
                    descriptionGuid = NEW_MENU_ITEMS[i - _newSoundOptionsIdx].DescriptionGuid;
                    cursorIdx = *NEW_MENU_ITEMS[i - _newSoundOptionsIdx].SettingPtr;
                    cursorMax = NEW_BGM_SELECTION_OPTIONS.size();

                    // build new cursor name list
                    // cursorNameList = new System.Array[newNumOptions]
                    cursorNameList = api->create_managed_array(arrayType, cursorMax);
                    assert(cursorNameList != nullptr);
                    cursorNameList->add_ref();

                    // fill new cursor name list
                    for (int j = 0; j < cursorMax; j++)
                    {
                        // set cursorNameList[j]
                        cursorNameList->invoke(
                            "set_Item",
                            {
                                (void *)(intptr_t)j,
                                (void *)&NEW_BGM_SELECTION_OPTIONS[j],
                            });
                    }
                }

                // set newNameList[i]
                newNameList->invoke(
                    "set_Item",
                    {
                        (void *)(intptr_t)i,
                        &nameGuid,
                    });

                // set newGuidList[i]
                newGuidList->invoke(
                    "set_Item",
                    {
                        (void *)(intptr_t)i,
                        &descriptionGuid,
                    });

                // set newCursorIndexList[i]
                newCursorIndexList->invoke(
                    "set_Item",
                    {
                        (void *)(intptr_t)i,
                        (void *)(intptr_t)cursorIdx,
                    });

                // set newCursorMaxList[i]
                newCursorMaxList->invoke(
                    "set_Item",
                    {
                        (void *)(intptr_t)i,
                        (void *)(intptr_t)cursorMax,
                    });

                // set newCursorNameListList[i]
                newCursorNameListList->invoke(
                    "set_Item",
                    {
                        (void *)(intptr_t)i,
                        cursorNameList,
                    });
            }

            // set menuTblOption.NameList
            nameList->release();
            *nameList_ptr = newNameList;

            // set menuTblOption.GuidList
            guidList->release();
            *guidList_ptr = newGuidList;

            // set menuTblOption._CursolIndex
            cursorIndexList->release();
            *cursorIndexList_ptr = newCursorIndexList;

            // set menuTblOption.CursolMax
            cursorMaxList->release();
            *cursorMaxList_ptr = newCursorMaxList;

            // set menuTblOption._CursolNameList
            cursorNameListList->release();
            *cursorNameListList_ptr = newCursorNameListList;
        },
        false);

    // Hook method which sets up the Audio menu
    reframework::API::Method *setupContentMethod = tdb->find_method("app.cLauncherOptionSound", "setupContent()");
    assert(setupContentMethod != nullptr);
    setupContentMethod->add_hook(
        [](int argc, void **argv, auto...)
        {
            _menuTblOption = (reframework::API::ManagedObject *)argv[1];

            // Set a flag to resize the options list after it has been initialized
            _doResizeSoundList = true;

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        nullptr, false);

    // Hook first TDB method which gets called after _SoundList is initialized in setupContent()
    // Here we change the number of items in the list
    reframework::API::Method *getItemInstTblMethod = tdb->find_method("via.gui.FluentScrollList", "get_ItemInstTbl");
    assert(getItemInstTblMethod != nullptr);
    getItemInstTblMethod->add_hook(
        [](int argc, void **argv, auto...)
        {
            if (!_doResizeSoundList)
            {
                return REFRAMEWORK_HOOK_CALL_ORIGINAL;
            }
            _doResizeSoundList = false;

            reframework::InvokeRet x;

            reframework::API::ManagedObject *soundList = (reframework::API::ManagedObject *)argv[1];
            assert(soundList != nullptr);

            // get soundList.ItemCount
            x = soundList->invoke(
                "get_ItemCount",
                {});
            _newSoundOptionsIdx = (int)x.dword;

            // set soundList.ItemCount
            soundList->invoke(
                "set_ItemCount",
                {
                    (void *)(intptr_t)(_newSoundOptionsIdx + NEW_MENU_ITEMS.size()),
                });

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        nullptr, false);

    // Hook method which is called every tick while in Audio menu
    // We basically need to replace this whole function with one which can handle list scrolling
    reframework::API::Method *doUpdateMethod = tdb->find_method("app.cLauncherOptionSound", "doUpdate()");
    assert(doUpdateMethod != nullptr);
    doUpdateMethod->add_hook(
        [](int argc, void **argv, auto...)
        {
            auto updateList = [](reframework::API::ManagedObject *menu)
            {
                auto &api = reframework::API::get();
                auto tdb = api->tdb();
                reframework::InvokeRet x;

                // get menu._SoundList
                reframework::API::ManagedObject **soundList_ptr = menu->get_field<reframework::API::ManagedObject *>("_SoundList");
                assert(soundList_ptr != nullptr);
                reframework::API::ManagedObject *soundList = *soundList_ptr;
                assert(soundList != nullptr);

                // call soundList.get_ItemInstTbl()
                x = soundList->invoke(
                    "get_ItemInstTbl",
                    {});
                reframework::API::ManagedObject *itemInstTbl = (reframework::API::ManagedObject *)x.ptr;
                assert(itemInstTbl != nullptr);

                // call itemInstTbl.get_Length()
                x = itemInstTbl->invoke(
                    "get_Length",
                    {});
                std::int32_t itemInstTblLength = x.dword;

                // get menu.NameList
                reframework::API::ManagedObject **nameList_ptr = menu->get_field<reframework::API::ManagedObject *>("NameList");
                assert(nameList_ptr != nullptr);
                reframework::API::ManagedObject *nameList = *nameList_ptr;
                assert(nameList != nullptr);

                // get menu._CursolIndex
                reframework::API::ManagedObject **cursorIndexList_ptr = menu->get_field<reframework::API::ManagedObject *>("_CursolIndex");
                assert(cursorIndexList_ptr != nullptr);
                reframework::API::ManagedObject *cursorIndexList = *cursorIndexList_ptr;
                assert(cursorIndexList != nullptr);

                // get menu.CursolMax
                reframework::API::ManagedObject **cursorMaxList_ptr = menu->get_field<reframework::API::ManagedObject *>("CursolMax");
                assert(cursorMaxList_ptr != nullptr);
                reframework::API::ManagedObject *cursorMaxList = *cursorMaxList_ptr;
                assert(cursorMaxList != nullptr);

                // get menu._CursolNameList
                reframework::API::ManagedObject **cursorNameListList_ptr = menu->get_field<reframework::API::ManagedObject *>("_CursolNameList");
                assert(cursorNameListList_ptr != nullptr);
                reframework::API::ManagedObject *cursorNameListList = *cursorNameListList_ptr;
                assert(cursorNameListList != nullptr);

                // get menu.VolumeInitValue
                reframework::API::ManagedObject **volumeInitValueList_ptr = menu->get_field<reframework::API::ManagedObject *>("VolumeInitValue");
                assert(volumeInitValueList_ptr != nullptr);
                reframework::API::ManagedObject *volumeInitValueList = *volumeInitValueList_ptr;
                assert(volumeInitValueList != nullptr);

                // call nameList.get_Length()
                x = nameList->invoke(
                    "get_Length",
                    {});
                std::int32_t nameList_Length = (std::int32_t)x.dword;

                // call cursorIndexList.get_Length()
                x = cursorIndexList->invoke(
                    "get_Length",
                    {});
                std::int32_t cursorIndexList_Length = (std::int32_t)x.dword;

                // call cursorMaxList.get_Length()
                x = cursorMaxList->invoke(
                    "get_Length",
                    {});
                std::int32_t cursorMaxList_Length = (std::int32_t)x.dword;

                // call cursorNameListList.get_Length()
                x = cursorNameListList->invoke(
                    "get_Length",
                    {});
                std::int32_t cursorNameListList_Length = (std::int32_t)x.dword;

                // call volumeInitValueList.get_Length()
                x = volumeInitValueList->invoke(
                    "get_Length",
                    {});
                std::int32_t volumeInitValueList_Length = (std::int32_t)x.dword;

                // get via.gui.asset.ui00030000._ITM_List_Item_._Pattern_Slider
                // get via.gui.asset.ui00030000._ITM_List_Item_._Pattern_LR_Selection
                reframework::API::TypeDefinition *itmListItemType = tdb->find_type("via.gui.asset.ui00030000._ITM_List_Item_");
                assert(itmListItemType != nullptr);
                reframework::API::Field *patternSliderField = itmListItemType->find_field("_Pattern_Slider");
                reframework::API::Field *patternLRSelectionField = itmListItemType->find_field("_Pattern_LR_Selection");
                assert(patternSliderField != nullptr);
                assert(patternLRSelectionField != nullptr);
                std::uint32_t *patternSlider_ptr = (std::uint32_t *)patternSliderField->get_data_raw(nullptr);
                std::uint32_t *patternLRSelection_ptr = (std::uint32_t *)patternLRSelectionField->get_data_raw(nullptr);
                assert(patternSlider_ptr != nullptr);
                assert(patternLRSelection_ptr != nullptr);
                std::uint32_t patternSlider = *patternSlider_ptr;
                std::uint32_t patternLRSelection = *patternLRSelection_ptr;

                // create temporary strings
                reframework::API::ManagedObject *str_MsgId_Option_List_Item_Name = api->create_managed_string(L"MsgId_Option_List_Item_Name");
                reframework::API::ManagedObject *str_ObjPath_PNL_Option_List_Item_Slider_Path = api->create_managed_string(L"ObjPath_PNL_Option_List_Item_Slider_Path");
                reframework::API::ManagedObject *str_Float_Slider_Value_Init = api->create_managed_string(L"Float_Slider_Value_Init");
                reframework::API::ManagedObject *str_Float_Slider_Value_Current = api->create_managed_string(L"Float_Slider_Value_Current");
                assert(str_MsgId_Option_List_Item_Name != nullptr);
                assert(str_ObjPath_PNL_Option_List_Item_Slider_Path != nullptr);
                assert(str_Float_Slider_Value_Init != nullptr);
                assert(str_Float_Slider_Value_Current != nullptr);

                for (std::size_t i = 0; i < itemInstTblLength; ++i)
                {
                    // get itemInstTbl[i]
                    x = itemInstTbl->invoke(
                        "get_Item",
                        {
                            (void *)(intptr_t)i,
                        });
                    reframework::API::ManagedObject *selectItem = (reframework::API::ManagedObject *)x.ptr;
                    assert(selectItem != nullptr);

                    // call selectItem.get_ListIndex()
                    x = selectItem->invoke(
                        "get_ListIndex",
                        {});
                    std::int32_t listIndex = (std::int32_t)x.dword;

                    // Normally the game will compute a MurmurHash3 of the string, but this is easier
                    // call selectItem.getParameterLegacy()
                    x = selectItem->invoke(
                        "getParameterLegacy(System.String)",
                        {
                            str_MsgId_Option_List_Item_Name,
                        });
                    reframework::API::ManagedObject *parameterVariable = (reframework::API::ManagedObject *)x.ptr;
                    assert(parameterVariable != nullptr);

                    // get nameList[listIndex]
                    assert(listIndex >= 0 && listIndex < nameList_Length);
                    x = nameList->invoke(
                        "get_Item",
                        {
                            (void *)(intptr_t)listIndex,
                        });
                    Guid nameGuid = *(Guid *)&x;

                    // get cursorNameListList[listIndex]
                    assert(listIndex >= 0 && listIndex <= cursorNameListList_Length);
                    x = cursorNameListList->invoke(
                        "get_Item",
                        {
                            (void *)(intptr_t)listIndex,
                        });
                    reframework::API::ManagedObject *cursorNameList = (reframework::API::ManagedObject *)x.ptr;

                    // call parameterVariable.set_ValueMessageId()
                    parameterVariable->invoke(
                        "set_ValueMessageId(System.Guid)",
                        {
                            &nameGuid,
                        });

                    // assume option is a slider type if it has no name list
                    if (cursorNameList == nullptr)
                    {
                        // call select.set_StatePattern()
                        selectItem->invoke(
                            "set_StatePattern(System.UInt32)",
                            {
                                (void *)(intptr_t)patternSlider,
                            });

                        // call selectItem.getParameterLegacy()
                        x = selectItem->invoke(
                            "getParameterLegacy(System.String)",
                            {
                                str_ObjPath_PNL_Option_List_Item_Slider_Path,
                            });
                        parameterVariable = (reframework::API::ManagedObject *)x.ptr;
                        assert(parameterVariable != nullptr);

                        // call parameterVariable.get_ValueObjectPath()
                        x = parameterVariable->invoke(
                            "get_ValueObjectPath",
                            {});
                        reframework::API::ManagedObject *objPath = (reframework::API::ManagedObject *)x.ptr;
                        assert(objPath != nullptr);

                        // call selectItem.getObject
                        // Technically the game calls getObject(System.String, System.Type),
                        // which we could do with tdb->find_type("via.gui.Panel")->get_type(),
                        // but we don't care about the strong typing anyway
                        x = selectItem->invoke(
                            "getObject(System.String)",
                            {
                                objPath,
                            });
                        reframework::API::ManagedObject *sliderPanel = (reframework::API::ManagedObject *)x.ptr;
                        assert(sliderPanel != nullptr);

                        // call sliderPanel.getParameterLegacy()
                        x = sliderPanel->invoke(
                            "getParameterLegacy(System.String)",
                            {
                                str_Float_Slider_Value_Init,
                            });
                        reframework::API::ManagedObject *paramVariable_floatSliderValueInit = (reframework::API::ManagedObject *)x.ptr;
                        assert(paramVariable_floatSliderValueInit != nullptr);

                        // get volumeInitValueList[listIndex]
                        assert(listIndex >= 0 && listIndex <= volumeInitValueList_Length);
                        x = volumeInitValueList->invoke(
                            "get_Item",
                            {
                                (void *)(intptr_t)listIndex,
                            });
                        double volumeInitValue = x.d;

                        // get cursorMaxList[listIndex]
                        assert(listIndex >= 0 && listIndex <= cursorMaxList_Length);
                        x = cursorMaxList->invoke(
                            "get_Item",
                            {
                                (void *)(intptr_t)listIndex,
                            });
                        std::int32_t cursorMax = (std::int32_t)x.dword;

                        // call paramVariable_floatSliderValueInit.set_ValueFloat()
                        double sliderValueInit = volumeInitValue / (cursorMax - 1) * 100.0;
                        paramVariable_floatSliderValueInit->invoke(
                            "set_ValueFloat(System.Single)",
                            {
                                (void *)(intptr_t)*reinterpret_cast<std::uint64_t *>(&sliderValueInit),
                            });

                        // call sliderPanel.getParameterLegacy()
                        x = sliderPanel->invoke(
                            "getParameterLegacy(System.String)",
                            {
                                str_Float_Slider_Value_Current,
                            });
                        reframework::API::ManagedObject *paramVariable_floatSliderValueCurrent = (reframework::API::ManagedObject *)x.ptr;
                        assert(paramVariable_floatSliderValueCurrent != nullptr);

                        // get cursorIndexList[listIndex]
                        assert(listIndex >= 0 && listIndex <= cursorIndexList_Length);
                        x = cursorIndexList->invoke(
                            "get_Item",
                            {
                                (void *)(intptr_t)listIndex,
                            });
                        std::int32_t cursorIndex = (std::int32_t)x.dword;

                        // call paramVariable_floatSliderValueCurrent.set_ValueFloat()
                        double sliderValueCurrent = (double)cursorIndex / (cursorMax - 1) * 100.0;
                        paramVariable_floatSliderValueCurrent->invoke(
                            "set_ValueFloat(System.Single)",
                            {
                                (void *)(intptr_t)*reinterpret_cast<std::uint64_t *>(&sliderValueCurrent),
                            });
                    }
                    else
                    {
                        // call select.set_StatePattern()
                        selectItem->invoke(
                            "set_StatePattern(System.UInt32)",
                            {
                                (void *)(intptr_t)patternLRSelection,
                            });
                    }
                }
            };
            auto updateGuidMessage = [](reframework::API::ManagedObject *menu)
            {
                reframework::InvokeRet x;

                // get menu.GuidList
                reframework::API::ManagedObject **guidList_ptr = menu->get_field<reframework::API::ManagedObject *>("GuidList");
                assert(guidList_ptr != nullptr);
                reframework::API::ManagedObject *guidList = *guidList_ptr;
                assert(guidList != nullptr);

                // get guidList.Length
                x = guidList->invoke(
                    "get_Length",
                    {});
                std::int32_t guidList_Length = (std::int32_t)x.dword;

                // get menu._SelectedIndex
                std::int32_t *selectedIndex_ptr = menu->get_field<std::int32_t>("_SelectedIndex");
                assert(selectedIndex_ptr != nullptr);
                std::int32_t selectedIndex = *selectedIndex_ptr;

                // get guidList[selectedIndex]
                assert(selectedIndex >= 0 && selectedIndex < guidList_Length);
                x = guidList->invoke(
                    "get_Item",
                    {
                        (void *)(intptr_t)selectedIndex,
                    });
                Guid guid = *(Guid *)&x;

                // call menu.setGuidMessage()
                menu->invoke(
                    "setGuidMessage",
                    {
                        &guid,
                    });
            };

            reframework::InvokeRet x;

            reframework::API::ManagedObject *menu = (reframework::API::ManagedObject *)argv[1];

            // get menu._SelectedIndex
            std::int32_t *selectedIndex_ptr = menu->get_field<std::int32_t>("_SelectedIndex");
            assert(selectedIndex_ptr != nullptr);

            // get menu._SoundList
            reframework::API::ManagedObject **soundList_ptr = menu->get_field<reframework::API::ManagedObject *>("_SoundList");
            assert(soundList_ptr != nullptr);
            reframework::API::ManagedObject *soundList = *soundList_ptr;
            assert(soundList != nullptr);

            // call menu.isUpdateList()
            // returns true if different item is selected and updates selectedIndex
            x = menu->invoke(
                "isUpdateList",
                {
                    selectedIndex_ptr, // passed by ref
                    (void *)(intptr_t)(_newSoundOptionsIdx + NEW_MENU_ITEMS.size()),
                });
            bool isUpdateList = x.byte;

            if (isUpdateList)
            {
                // call soundList.set_SelectedIndex
                soundList->invoke(
                    "set_SelectedIndex",
                    {
                        (void *)(intptr_t)*selectedIndex_ptr,
                    });

                updateList(menu);
                updateGuidMessage(menu);
                return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
            }

            // call menu.isUpdateListPage()
            // returns true if list is page scrolled and updates selectedIndex
            // also calls soundList.set_SelectedIndex internally
            x = menu->invoke(
                "isUpdateListPage",
                {
                    soundList,
                    selectedIndex_ptr, // passed by ref
                });
            bool isUpdateListPage = x.byte;

            if (isUpdateListPage)
            {
                updateList(menu);
                updateGuidMessage(menu);
                return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
            }

            return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
        },
        nullptr, false);

    return true;
}
