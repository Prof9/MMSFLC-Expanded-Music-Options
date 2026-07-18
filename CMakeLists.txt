cmake_minimum_required(
	VERSION 3.20.0
)

project(
	ExpandedMusicOptions
	VERSION 1.0.0
	LANGUAGES CXX
)
set(CACHE{GAME_INSTALL_EXE} VALUE "MEGAMAN_STARFORCE_Legacy_Collection.exe")
set(CACHE{GAME_INSTALL_DIR_NAME} VALUE "MegaMan StarForce LegacyCollection 1.0.2")

cmake_path(SET STEAM_LIBRARY_DIR "C:\\Program Files (x86)\\Steam\\")
cmake_path(SET GAME_INSTALL_DIR "${STEAM_LIBRARY_DIR}/steamapps/common/${GAME_INSTALL_DIR_NAME}/")

set(CACHE{STEAM_LIBRARY_DIR} VALUE "${STEAM_LIBRARY_DIR}")
set(CACHE{GAME_INSTALL_DIR} VALUE "${GAME_INSTALL_DIR}")

include_directories(SYSTEM
	"generated"
	"src"
	"modules/reframework/include"
)
add_library(REF_PLUGIN SHARED
	"src/AudioMenuExtension.cpp"
	"src/CustomPlaylistMenuItem.cpp"
	"src/Guid.cpp"
	"src/Main.cpp"
	"src/MenuItem.cpp"
	"src/MessageFile.cpp"
	"src/MessageManager.cpp"
	"src/MessageReference.cpp"
	"src/MessageUtility.cpp"
	"src/MusicSystemMod.cpp"
	"src/MusicSystemMod_Hooks.cpp"
	"src/MusicSystemMod_Menu.cpp"
	"src/MusicSystemMod_Settings.cpp"
	"src/MusicSystemMod_Strings.cpp"
	"src/REFrameworkHelper.cpp"
)

add_definitions(
	-DUNICODE
)
target_compile_features(REF_PLUGIN PUBLIC
	cxx_std_23
)

set_target_properties(REF_PLUGIN PROPERTIES
	OUTPUT_NAME ${PROJECT_NAME}
)

# Sanitize extracted Enums_Internal.hpp (change enum to enum class, deduplicate enum names)
find_package(Python 3.8 REQUIRED)
execute_process(
	COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/sanitize_enums.py" "${GAME_INSTALL_DIR}/Enums_Internal.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/Enums_Internal.hpp"
)
