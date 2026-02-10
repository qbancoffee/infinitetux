cmake_minimum_required(VERSION 3.16)
project(InfiniteTux VERSION 1.0.0 LANGUAGES CXX)

# Runtime data root (packagers can override)
set(INFINITE_TUX_DATADIR "" CACHE STRING "Prefix path used at runtime to locate resources/ (should end with a slash)")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Art.cpp
    src/Scene.cpp
    src/TitleScene.cpp
    src/MapScene.cpp
    src/LevelScene.cpp
    src/WinScene.cpp
    src/LoseScene.cpp
    src/OptionsScene.cpp
    src/InputConfig.cpp
    src/Level.cpp
    src/LevelGenerator.cpp
    src/Sprite.cpp
    src/Mario.cpp
    src/Enemy.cpp
    src/FlowerEnemy.cpp
    src/Shell.cpp
    src/Fireball.cpp
    src/Mushroom.cpp
    src/FireFlower.cpp
    src/Particle.cpp
    src/Sparkle.cpp
    src/CoinAnim.cpp
    src/BulletBill.cpp
    src/LevelRenderer.cpp
    src/BgRenderer.cpp
    src/SpriteTemplate.cpp
    src/ImprovedNoise.cpp
)

# Platform-specific configuration
if(WIN32)
    # Windows build (native or cross-compiled)
    # If SDL2 paths are already set (e.g., from toolchain file), use them
    if(NOT SDL2_FOUND)
        # Try to find SDL2 using CMake config mode first, then fall back to manual
        find_package(SDL2 CONFIG QUIET)
        find_package(SDL2_image CONFIG QUIET)
        find_package(SDL2_mixer CONFIG QUIET)
        
        if(NOT SDL2_FOUND)
            # Manual SDL2 finding for MinGW cross-compilation
            find_path(SDL2_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL2 include/SDL2)
            find_library(SDL2_LIBRARY NAMES SDL2 SDL2main)
            find_library(SDL2_IMAGE_LIBRARY NAMES SDL2_image)
            find_library(SDL2_MIXER_LIBRARY NAMES SDL2_mixer)
            
            if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
                set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
                set(SDL2_LIBRARIES ${SDL2_LIBRARY})
                set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARY})
                set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
            endif()
        else()
            set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
            set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
            set(SDL2_IMAGE_LIBRARIES SDL2_image::SDL2_image)
            set(SDL2_MIXER_LIBRARIES SDL2_mixer::SDL2_mixer)
        endif()
    endif()
    
    # Create executable
    # We use SDL_MAIN_HANDLED so we don't need SDL2main or mingw32
    # Note: -mwindows removed temporarily for debugging (hides console)
    add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "infinitetux")

    
    # Disable stack protector - can cause issues with Wine
    target_compile_options(${PROJECT_NAME} PRIVATE -fno-stack-protector)
    
    # Link Windows-specific libraries
    target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        shell32
        ole32
    )
    
elseif(APPLE)
    # macOS build
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    
    add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "infinitetux")
    
    target_link_libraries(${PROJECT_NAME}
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_mixer::SDL2_mixer
    )
    
else()
    # Linux build - use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
    
    add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "infinitetux")
    
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
    )
    
    target_link_directories(${PROJECT_NAME} PRIVATE
        ${SDL2_LIBRARY_DIRS}
        ${SDL2_IMAGE_LIBRARY_DIRS}
        ${SDL2_MIXER_LIBRARY_DIRS}
    )
    
    target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
    )
endif()

# Include directories (common)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
)

# Pass INFINITE_TUX_DATADIR to the compiler as a preprocessor definition
if(NOT "${INFINITE_TUX_DATADIR}" STREQUAL "")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        INFINITE_TUX_DATADIR="${INFINITE_TUX_DATADIR}"
    )
endif()

# Copy resources to build directory
file(COPY ${PROJECT_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

# Install rules
install(TARGETS ${PROJECT_NAME} DESTINATION games)
install(DIRECTORY resources/ DESTINATION share/games/infinitetux/resources)
