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
static bool _inSoundDoUpdate = false;
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
    // Here we modify the layout of the damage reduction setting to add additional rates
    reframework::API::Method *startMethod = tdb->find_method("app.GUILauncherOption", "start()");
    assert(startMethod != nullptr);
    startMethod->add_hook(
        [](auto...)
        {
            /*
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

            // get menuTblOption.CursolName
            reframework::API::ManagedObject **cursorNameListList_ptr = _menuTblOption->get_field<reframework::API::ManagedObject *>("_CursolNameList");
            assert(cursorNameListList_ptr != nullptr);
            /*
            reframework::API::ManagedObject *cursorNameListList = *cursorNameListList_ptr;
            assert(cursorNameListList != nullptr);
            */
            /*

          // get nameList.Length
          x = nameList->invoke(
              "get_Length",
              {});
          int numOptions = (int)x.dword;

          // get GUID for BGM Selection string
          Guid *guidBgmSelection = MessageUtility::getMessageGuidByName("Common_MsgGUI00030000_Sound_5");
          assert(guidBgmSelection != nullptr);

          // find the BGM Selection option in the menu
          for (int i = 0; i < numOptions; ++i)
          {
              // get nameList[i]
              x = nameList->invoke(
                  "get_Item",
                  {
                      (void *)(intptr_t)i,
                  });
              Guid *nameListI = (Guid *)&x;

              if (*nameListI == *guidBgmSelection)
              {
                  bgmSelectionIdx = i;
                  break;
              }
          }
          assert(bgmSelectionIdx != -1);

          // Remove the existing items and put in the new items
          int newNumOptions = numOptions - 1 + NEW_MENU_ITEMS.size();

          /// TODO: Check menu already extended

          // get System.Guid type
          reframework::API::TypeDefinition *guidType = tdb->find_type("System.Guid");
          assert(guidType != nullptr);

          // get System.Int32 type
          reframework::API::TypeDefinition *int32Type = tdb->find_type("System.Int32");
          assert(int32Type != nullptr);

          // get System.Array type
          reframework::API::TypeDefinition *arrayType = tdb->find_type("System.Array");
          assert(arrayType != nullptr);

          // newNameList = new System.Guid[newNumOptions]
          reframework::API::ManagedObject *newNameList = api->create_managed_array(guidType, newNumOptions);
          assert(newNameList != nullptr);
          newNameList->add_ref();

          // newGuidList = new System.Guid[newNumOptions]
          reframework::API::ManagedObject *newGuidList = api->create_managed_array(guidType, newNumOptions);
          assert(newGuidList != nullptr);
          newGuidList->add_ref();

          // newCursorIndexList = new System.Int32[newNumOptions]
          reframework::API::ManagedObject *newCursorIndexList = api->create_managed_array(int32Type, newNumOptions);
          assert(newCursorIndexList != nullptr);
          newCursorIndexList->add_ref();

          // newCursorMaxList = new System.Int32[newNumOptions]
          reframework::API::ManagedObject *newCursorMaxList = api->create_managed_array(int32Type, newNumOptions);
          assert(newCursorMaxList != nullptr);
          newCursorMaxList->add_ref();

          // newCursorNameListList = new System.Array[newNumOptions]
          reframework::API::ManagedObject *newCursorNameListList = api->create_managed_array(arrayType, newNumOptions);
          assert(newCursorNameListList != nullptr);
          newCursorNameListList->add_ref();

          // delete cursorNameListList[bgmSelectionIdx]
          /*
          x = cursorNameListList->invoke(
              "get_Item",
              {
                  (void *)(intptr_t)bgmSelectionIdx,
              });
          reframework::API::ManagedObject *cursorNameList = (reframework::API::ManagedObject *)x.ptr;
          assert(cursorNameList != nullptr);
          if (cursorNameList != nullptr)
          {
              cursorNameList->release();
          }
          */ /*

           // Fill new arrays
           for (int newIdx = 0; newIdx < newNumOptions; ++newIdx)
           {
               Guid nameGuid;
               Guid descriptionGuid;
               std::int32_t cursorIdx;
               std::int32_t cursorMax;
               reframework::API::ManagedObject *cursorNameList = nullptr;

               int oldIdx;
               if (newIdx < bgmSelectionIdx)
               {
                   oldIdx = newIdx;
               }
               else if (newIdx < bgmSelectionIdx + NEW_MENU_ITEMS.size())
               {
                   oldIdx = -1;
               }
               else
               {
                   oldIdx = newIdx - (newNumOptions - numOptions);
               }

               if (oldIdx != -1)
               {
                   // get nameList[oldIdx]
                   x = nameList->invoke(
                       "get_Item",
                       {
                           (void *)(intptr_t)oldIdx,
                       });
                   nameGuid = *(Guid *)&x;

                   // get guidList[oldIdx]
                   x = guidList->invoke(
                       "get_Item",
                       {
                           (void *)(intptr_t)oldIdx,
                       });
                   descriptionGuid = *(Guid *)&x;

                   // get cursorIndexList[oldIdx]
                   x = cursorIndexList->invoke(
                       "get_Item",
                       {
                           (void *)(intptr_t)oldIdx,
                       });
                   cursorIdx = (int32_t)x.dword;

                   // get cursorMaxList[oldIdx]
                   x = cursorMaxList->invoke(
                       "get_Item",
                       {
                           (void *)(intptr_t)oldIdx,
                       });
                   cursorMax = (int32_t)x.dword;

                   /*
                   // get cursorNameListList[oldIdx]
                   x = cursorNameListList->invoke(
                       "get_Item",
                       {
                           (void *)(intptr_t)oldIdx,
                       });
                   cursorNameList = (reframework::API::ManagedObject *)x.ptr;
                   */
            /*
        }
        else
        {
            // get new item
            nameGuid = NEW_MENU_ITEMS[newIdx - bgmSelectionIdx].NameGuid;
            descriptionGuid = NEW_MENU_ITEMS[newIdx - bgmSelectionIdx].DescriptionGuid;
            cursorIdx = *NEW_MENU_ITEMS[newIdx - bgmSelectionIdx].SettingPtr;
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
                        &NEW_BGM_SELECTION_OPTIONS[j],
                    });
                }
            }

            // set newNameList[newIdx]
            newNameList->invoke(
            "set_Item",
            {
                (void *)(intptr_t)newIdx,
                &nameGuid,
            });

            // set newGuidList[newIdx]
            newGuidList->invoke(
            "set_Item",
            {
                (void *)(intptr_t)newIdx,
                &descriptionGuid,
            });

            // set newCursorIndexList[newIdx]
            newCursorIndexList->invoke(
            "set_Item",
            {
                (void *)(intptr_t)newIdx,
                (void *)(intptr_t)cursorIdx,
            });

            // set newCursorMaxList[newIdx]
            newCursorMaxList->invoke(
            "set_Item",
            {
                (void *)(intptr_t)newIdx,
                (void *)(intptr_t)cursorMax,
            });

            // set newCursorNameListList[newIdx]
            newCursorNameListList->invoke(
            "set_Item",
            {
                (void *)(intptr_t)newIdx,
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
        */

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        nullptr,
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
    // Set a flag that we are updating the Audio menu (needed for other hooks)
    reframework::API::Method *doUpdateMethod = tdb->find_method("app.cLauncherOptionSound", "doUpdate()");
    assert(doUpdateMethod != nullptr);
    doUpdateMethod->add_hook(
        [](auto...)
        {
            _inSoundDoUpdate = true;

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        [](auto...)
        {
            _inSoundDoUpdate = false;
        },
        false);

    // Hook method which checks if list needs to be updated
    // Override size of list (this is hardcoded)
    reframework::API::Method *isUpdateListMethod = tdb->find_method("app.cLauncherOptionMenuBase", "isUpdateList(System.Int32, System.Int32)");
    assert(isUpdateListMethod != nullptr);
    isUpdateListMethod->add_hook(
        [](int argc, void **argv, auto...)
        {
            if (!_inSoundDoUpdate)
            {
                return REFRAMEWORK_HOOK_CALL_ORIGINAL;
            }

            // Override size of list
            *(int32_t *)&argv[3] = _newSoundOptionsIdx + NEW_MENU_ITEMS.size();

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        nullptr, false);

    // Hook method which is called when guide message changes
    // This can indicate the list was scrolled
    reframework::API::Method *setGuidMessageMethod = tdb->find_method("app.cLauncherOptionMenuBase", "setGuidMessage(System.Guid)");
    assert(setGuidMessageMethod != nullptr);
    setGuidMessageMethod->add_hook(
        [](int argc, void **argv, auto...)
        {
            if (!_inSoundDoUpdate)
            {
                return REFRAMEWORK_HOOK_CALL_ORIGINAL;
            }

            reframework::InvokeRet x;
            auto &api = reframework::API::get();
            auto tdb = api->tdb();

            reframework::API::ManagedObject *menu = (reframework::API::ManagedObject *)argv[1];

            // Update list
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

            // call nameList.get_Length()
            x = nameList->invoke(
                "get_Length",
                {});
            std::int32_t nameListLength = (std::int32_t)x.dword;

            // create temporary string
            reframework::API::ManagedObject *str_MsgId_Option_List_Item_Name = api->create_managed_string(L"MsgId_Option_List_Item_Name");
            assert(str_MsgId_Option_List_Item_Name != nullptr);

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

                Guid nameGuid;
                if (listIndex >= _newSoundOptionsIdx && listIndex < _newSoundOptionsIdx + NEW_MENU_ITEMS.size())
                {
                    // One of our new options
                    nameGuid = NEW_MENU_ITEMS[listIndex - _newSoundOptionsIdx].NameGuid;
                }
                else if (listIndex >= 0 && listIndex < nameListLength)
                {
                    // One of the existing options
                    // get nameList[selectedIndex]
                    x = nameList->invoke(
                        "get_Item",
                        {
                            (void *)(intptr_t)listIndex,
                        });
                    nameGuid = *(Guid *)&x;
                }
                else
                {
                    // New option that wasn't added by us, skip
                    continue;
                }

                // call parameterVariable.set_ValueMessageId()
                parameterVariable->invoke(
                    "set_ValueMessageId(System.Guid)",
                    {
                        &nameGuid,
                    });
            }

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        nullptr, false);

    // Hook method which is called to check if selected item changed value
    // We need to insert another call that checks if the page scrolled
    reframework::API::Method *isUpdateSelectedItemMethod = tdb->find_method("app.cLauncherOptionMenuBase", "isUpdateSelectedItem(System.Int32, System.Int32, System.Boolean)");
    assert(isUpdateSelectedItemMethod != nullptr);
    isUpdateSelectedItemMethod->add_hook(
        [](int argc, void **argv, auto...)
        {
            _isUpdateListPage = false;

            if (!_inSoundDoUpdate)
            {
                return REFRAMEWORK_HOOK_CALL_ORIGINAL;
            }

            reframework::InvokeRet x;

            reframework::API::ManagedObject *menu = (reframework::API::ManagedObject *)argv[1];

            // get menu._SoundList
            reframework::API::ManagedObject **soundList_ptr = menu->get_field<reframework::API::ManagedObject *>("_SoundList");
            assert(soundList_ptr != nullptr);
            reframework::API::ManagedObject *soundList = *soundList_ptr;
            assert(soundList != nullptr);

            // get menu.GuidList
            reframework::API::ManagedObject **guidList_ptr = menu->get_field<reframework::API::ManagedObject *>("GuidList");
            assert(guidList_ptr != nullptr);
            reframework::API::ManagedObject *guidList = *guidList_ptr;
            assert(guidList != nullptr);

            // call guidList.get_Length()
            x = guidList->invoke(
                "get_Length",
                {});
            std::int32_t guidListLength = (std::int32_t)x.dword;

            // get &menu._SelectedIndex
            std::int32_t *selectedIndex_ptr = menu->get_field<std::int32_t>("_SelectedIndex");
            assert(selectedIndex_ptr != nullptr);

            // call menu.isUpdateListPage()
            x = menu->invoke(
                "isUpdateListPage(via.gui.FluentScrollList, System.Int32)",
                {
                    soundList,
                    selectedIndex_ptr, // passed by ref
                });
            _isUpdateListPage = x.byte;

            if (_isUpdateListPage)
            {
                // Select appropriate description GUID
                // get menu._SelectedIndex
                std::int32_t selectedIndex = *selectedIndex_ptr;

                Guid descriptionGuid;
                if (selectedIndex >= _newSoundOptionsIdx && selectedIndex < _newSoundOptionsIdx + NEW_MENU_ITEMS.size())
                {
                    // One of our new options
                    descriptionGuid = NEW_MENU_ITEMS[selectedIndex - _newSoundOptionsIdx].DescriptionGuid;
                }
                else if (selectedIndex < guidListLength)
                {
                    // One of the existing options
                    // get guidList[selectedIndex]
                    x = guidList->invoke(
                        "get_Item",
                        {
                            (void *)(intptr_t)selectedIndex,
                        });
                    descriptionGuid = *(Guid *)&x;
                }
                else
                {
                    // New option that wasn't added by us, skip
                    return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
                }

                // call menu.setGuidMessage()
                // This will call the hooked version which will also call our updateList()
                menu->invoke(
                    "setGuidMessage(System.Guid)",
                    {
                        &descriptionGuid,
                    });

                // Return nullptr in post-hook, this will cause doUpdate() to exit
                return REFRAMEWORK_HOOK_SKIP_ORIGINAL;
            }

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        [](void **ret_val, auto...)
        {
            if (_isUpdateListPage)
            {
                // Override return value with nullptr, this will cause doUpdate() to exit
                *ret_val = nullptr;
                _isUpdateListPage = false;
            }
        },
        false);

    // Hook method which is called when the game populates the Options menu
    // with the actual saved settings from the player's save file
    // Here we overwrite the selected index for damage reduction with our custom rate
    reframework::API::Method *getSaveParamMethod = tdb->find_method("app.cLauncherOptionSound", "getSaveParam()");
    assert(getSaveParamMethod != nullptr);
    getSaveParamMethod->add_hook(
        nullptr,
        [](auto...)
        {
            /*
            reframework::InvokeRet x;

            // get launcherOptionDifficult._CursolIndex
            reframework::API::ManagedObject **cursorIndex_ptr = _menuTblOption->get_field<reframework::API::ManagedObject *>("_CursolIndex");
            assert(cursorIndex_ptr != nullptr);
            reframework::API::ManagedObject *cursorIndex = *cursorIndex_ptr;
            assert(cursorIndex != nullptr);

            // set _CursolIndex[damageReductionIdx]
            x = cursorIndex->invoke(
                "set_Item",
                {
                    (void *)(intptr_t)_damageReductionIdx,
                    (void *)(intptr_t)GetClosestDamageMultiplierIdx(),
                });
            */
        },
        false);

    // Hook "Reset Settings" option used inside Difficulty tab
    // Here we reset our custom damage multiplier to 100%
    // This is needed because our 100% multiplier is in a different list index
    // than the original 0% reduction
    reframework::API::Method *resetSettingMethod = tdb->find_method("app.cLauncherOptionDifficult", "resetSetting()");
    assert(resetSettingMethod != nullptr);
    resetSettingMethod->add_hook(
        [](auto...)
        {
            /*
            _damageMultiplier = DEFAULT_DAMAGE_MULTIPLIER;
            */

            return REFRAMEWORK_HOOK_CALL_ORIGINAL;
        },
        [](auto...)
        {
            /*
            reframework::InvokeRet x;

            // get launcherOptionDifficult._CursolIndex
            reframework::API::ManagedObject **cursorIndex_ptr = _menuTblOption->get_field<reframework::API::ManagedObject *>("_CursolIndex");
            assert(cursorIndex_ptr != nullptr);
            reframework::API::ManagedObject *cursorIndex = *cursorIndex_ptr;
            assert(cursorIndex != nullptr);

            // set _CursolIndex[damageReductionIdx]
            x = cursorIndex->invoke(
                "set_Item",
                {
                    (void *)(intptr_t)_damageReductionIdx,
                    (void *)(intptr_t)GetClosestDamageMultiplierIdx(),
                });
            */
        },
        false);

    return true;
}
