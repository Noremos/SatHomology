cmake_minimum_required(VERSION 3.28)
project(SatHomology CXX)

# Turning off extensions avoids an issue with the clang 16 compiler
# clang 17 and greater can avoid this setting
set(CMAKE_CXX_EXTENSIONS OFF)
# Set the version of C++ for the project
set(CMAKE_CXX_STANDARD 20)
# Create a library
set(CMAKE_COLOR_DIAGNOSTICS ON)

file(GLOB_RECURSE SOURCES
    "frontend/*.cpp"
    "backend/*.cpp"
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
)


file(GLOB_RECURSE SOURCES_MODULES
    "Bind/*.ixx"
    "frontend/*.ixx"
    "backend/*.ixx"
    # "side/Barcode/PrjBarlib/modules/*.ixx"
    "side/Barcode/PrjBarlib/modules/StateBinFile.ixx"
    # "side/Barcode/PrjBarlib/modules/TiffReader.ixx"
)

list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*backend/Algs/CachedBarcodeAlg\\.ixx$")
list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*backend/Interfaces/IAlg\\.ixx$")
list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*backend/Layers/SimpleLineDisplay\\.ixx$")
list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*backend/Layers/SimpleLineDisplay\\.ixx$")
list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*frontend/Viewer3d\\.ixx$")
list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*frontend/Object3d\\.ixx$")
list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*side/Barcode/PrjBarlib/modules/BinStateFileWrapper\\.ixx$")
list(FILTER SOURCES_MODULES EXCLUDE REGEX ".*side/Barcode/PrjBarlib/modules/ImageReader\\.ixx$")


# add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)

include_directories(
    side
    side/Barcode/PrjBarlib/modules
    side/imgui
    Bind
)

# Настройка конфигураций
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CXX_SCAN_FOR_MODULES TRUE)

# Добавление исполняемого файла
add_executable(SatHomology ${SOURCES} ${SIDE_SOURCES})
target_sources(SatHomology
  PUBLIC
    FILE_SET all_my_modules TYPE CXX_MODULES FILES
    ${SOURCES_MODULES}
)


set_target_properties(SatHomology PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
)


# add_library(foo)
# # Add the module file to the library
# target_sources(foo
#   PUBLIC
#     FILE_SET CXX_MODULES FILES
#       foo.cxx
# )



# this is heuristically generated, and may not be correct
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(SatHomology PRIVATE glfw)

# this is heuristically generated, and may not be correct
find_package(jsoncpp CONFIG REQUIRED)
target_link_libraries(SatHomology PRIVATE JsonCpp::JsonCpp)

find_package(Lua REQUIRED)
target_include_directories(SatHomology PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(SatHomology PRIVATE ${LUA_LIBRARIES})

find_package(unofficial-sqlite3 CONFIG REQUIRED)
target_link_libraries(SatHomology PRIVATE unofficial::sqlite3::sqlite3)

find_package(GLEW REQUIRED)
target_link_libraries(SatHomology PRIVATE GLEW::GLEW)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(SatHomology PRIVATE DEBUG)
    target_compile_options(SatHomology PRIVATE -g)
else()
    target_compile_definitions(SatHomology PRIVATE NDEBUG)
    target_compile_options(SatHomology PRIVATE -O2)
endif()

file(GLOB RES_FILES
  "Build/Resurses/*"
)
file(COPY ${RES_FILES} DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
