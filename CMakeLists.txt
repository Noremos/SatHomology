cmake_minimum_required(VERSION 3.28)
if (BUILD_TEST)
    set(BTARGET SatTests)
else()
    set(BTARGET SatHomology)
endif()
project(${BTARGET})

# Turning off extensions avoids an issue with the clang 16 compiler
# clang 17 and greater can avoid this setting
set(CMAKE_CXX_EXTENSIONS OFF)
# Set the version of C++ for the project
set(CMAKE_CXX_STANDARD 20)
# Create a library
set(CMAKE_COLOR_DIAGNOSTICS ON)
# set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX14.sdk)

file(GLOB_RECURSE SOURCES
    "frontend/*.cpp"
    "backend/*.cpp"
    "Bind/*.cpp"
    "side/imgui/backends/imgui_impl_glfw.cpp"
    "side/imgui/backends/imgui_impl_opengl3.cpp"
    "side/barcode/PrjBarlib/source/*.cpp"
)
list(FILTER SOURCES EXCLUDE REGEX ".*barcodeCreatorCloud\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*Source\\.cpp$")

set(SIDE_SOURCES
    side/sago/platform_folders.cpp
    side/implot/implot_demo.cpp
    side/implot/implot_items.cpp
    side/implot/implot.cpp
    side/imgui/imgui.cpp
    side/imgui/imgui_demo.cpp
    side/imgui/imgui_draw.cpp
    side/imgui/imgui_tables.cpp
    side/imgui/imgui_widgets.cpp
    side/fpng/fpng.cpp
    side/barcode/PrjBarlib/modules/tiffreader.cpp
    side/Hungarian.cpp
    side/hclust-cpp/fastcluster.cpp
    side/tinyfiledialogs.c
    side/Barcode/PrjBarlib/extra/BarcodeCreatorCloud.cpp
)



# list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*backend/Algs/CachedBarcodeAlg\\.ixx$")
# list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*backend/Interfaces/IAlg\\.ixx$")
# list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*backend/Layers/SimpleLineDisplay\\.ixx$")
# list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*backend/Layers/SimpleLineDisplay\\.ixx$")
# list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*frontend/Viewer3d\\.ixx$")
# list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*frontend/Object3d\\.ixx$")
# list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*side/Barcode/PrjBarlib/modules/BinStateFileWrapper\\.ixx$")
# list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*side/Barcode/PrjBarlib/modules/ImageReader\\.ixx$")


# add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)

include_directories(
    side
    side/Barcode/PrjBarlib/modules
    side/imgui
    Bind
)

# Настройка конфигураций
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
# set(CXX_SCAN_FOR_MODULES TRUE)

if (BUILD_TEST)
    enable_testing()

    list(FILTER SOURCES EXCLUDE REGEX ".*\\Application\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*\\Application\\.h$")
    list(FILTER SOURCES EXCLUDE REGEX ".*\\main\\.cpp$")

    find_package(GTest REQUIRED)
    add_executable(${BTARGET}
        ${SOURCES}
        ${SIDE_SOURCES}
        # Tests/FindClosestTests.cpp
        Tests/FindCounturTests.cpp
        # Tests/SatTests.cpp
    )

    set_target_properties(${BTARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../
    )

    target_include_directories(${BTARGET}
        PUBLIC
        Tests/googletest)

    target_link_libraries(${BTARGET} PUBLIC GTest::GTest GTest::Main)

    target_compile_definitions(${BTARGET} PRIVATE DEBUG)
    target_compile_options(${BTARGET} PRIVATE -g)

    include(GoogleTest)
    gtest_discover_tests(${BTARGET})
else()
    # Добавление исполняемого файла
    add_executable(${BTARGET} ${SOURCES} ${SIDE_SOURCES})



    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(OUT_FOLTE "Debug")

        target_compile_definitions(${BTARGET} PRIVATE DEBUG)
        target_compile_options(${BTARGET} PRIVATE -g)
    else()
        set(OUT_FOLTE "Release")

        target_compile_definitions(${BTARGET} PRIVATE NDEBUG)
        target_compile_options(${BTARGET} PRIVATE -O3 -Wall -DNDEBUG -g)
    endif()

    set_target_properties(${BTARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../../${OUT_FOLTE}/
    )

    file(GLOB RES_FILES
     "Build/Resurses/*"
    )
    file(COPY ${RES_FILES} DESTINATION ${CMAKE_BINARY_DIR}/../../${OUT_FOLTE}/)
endif()

# target_sources(${BTARGET}
#     PUBLIC
#     FILE_SET all_my_modules TYPE CXX_MODULES FILES
#     ${SOURCES_MODULES}
# )

# add_library(foo)
# # Add the module file to the library
# target_sources(foo
#   PUBLIC
#     FILE_SET CXX_MODULES FILES
#       foo.cxx
# )

# this is heuristically generated, and may not be correct
find_package(shapelib CONFIG REQUIRED)
target_link_libraries(${BTARGET} PRIVATE shapelib::shp)

# this is heuristically generated, and may not be correct
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${BTARGET} PRIVATE glfw)

find_package(Lua REQUIRED)
target_include_directories(${BTARGET} PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(${BTARGET} PRIVATE ${LUA_LIBRARIES})

find_package(unofficial-sqlite3 CONFIG REQUIRED)
target_link_libraries(${BTARGET} PRIVATE unofficial::sqlite3::sqlite3)

find_package(GLEW REQUIRED)
target_link_libraries(${BTARGET} PRIVATE GLEW::GLEW)
