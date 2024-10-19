# Copyright (c) the Dviglo project
# License: MIT

# Указываем минимальную версию CMake
cmake_minimum_required(VERSION 3.16)

# Если используется одноконфигурационный генератор
# и конфигурация не указана
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    # то конфигурацией по умолчанию будет Release
    set(CMAKE_BUILD_TYPE Release)

    # Нельзя оставлять переменную CMAKE_BUILD_TYPE пустой,
    # так как при этом не будут заданы флаги GCC и MinGW:
    # * Пустая строка: CXX_FLAGS = -std=c++20
    # * Release: CXX_FLAGS = -O3 -DNDEBUG -std=c++20
    # * Debug: CXX_FLAGS = -g -std=c++20
    # * RelWithDebInfo: CXX_FLAGS = -O2 -g -DNDEBUG -std=c++20
    # * MinSizeRel: CXX_FLAGS = -Os -DNDEBUG -std=c++20
    # Флаги можно посмотреть в файле build/source/dviglo/CMakeFiles/dviglo.dir/flags.make
endif()

# Включаем поддержку папок в IDE
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Подключаем директорию с библиотекой
add_subdirectory(third_party/stb)

# Название проекта
project(sprite_manipulator)

# Версия стандарта C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Указываем Студии на то, что исходники в кодировке UTF-8.
# Это позволяет писать U'🍌'. У других компиляторов, похоже, нет с этим проблем.
# https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2295r6.pdf
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Название таргета
set(target_name sprite_manipulator)

# Создаём список файлов
file(GLOB_RECURSE source_files src/*.cpp src/*.hpp)

# Создаём консольное приложение
add_executable(${target_name} ${source_files})

# Подключаем библиотеку
target_link_libraries(${target_name} PRIVATE stb)

# Отладочная версия приложения будет иметь суффикс _d
set_property(TARGET ${target_name} PROPERTY DEBUG_POSTFIX _d)

# Заставляем Visual Studio отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${source_files})

# В Visual Studio таргет будет назначен стартовым вместо ALL_BUILD,
# чтобы потом не делать это вручную при отладке игры
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target_name})
