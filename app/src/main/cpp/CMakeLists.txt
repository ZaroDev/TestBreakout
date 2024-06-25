# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.22.1)

project("testbreakout")


file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/breakout/*.cpp")
# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/freetype/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/breakout")


add_library(freetype STATIC IMPORTED)
add_library(testbreakout SHARED ${MY_SOURCES})


set_target_properties(freetype PROPERTIES IMPORTED_LOCATION
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/freetype/${ANDROID_ABI}/libfreetype2-static.a")
# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

# Configure libraries CMake uses to link your target library.
target_link_libraries(testbreakout
        # The game activity
        game-activity::game-activity

        # EGL and other dependent libraries required for drawing
        # and interacting with Android system
        EGL
        GLESv3
        jnigraphics
        android
        log
        freetype)