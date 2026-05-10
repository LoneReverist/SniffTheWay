cmake_minimum_required(VERSION 3.29)

project(SniffTheWay)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else() # GCC/Clang
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif()

# Configure DreamhearthEngine to only build Vulkan support
set(BUILD_VULKAN ON)
set(BUILD_OPENGL OFF)
set(BUILD_DEMOS OFF)
add_subdirectory(DreamhearthEngine)

add_executable(SniffTheWay)

set_target_properties(SniffTheWay PROPERTIES CXX_SCAN_FOR_MODULES ON)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE MODULE_FILES "src/*.ixx")

target_sources(SniffTheWay PRIVATE
    FILE_SET cxx_modules TYPE CXX_MODULES
    BASE_DIRS src
    FILES ${MODULE_FILES}
    PRIVATE ${SOURCE_FILES}
)

target_link_libraries(SniffTheWay PRIVATE
    GlfwVkWindow
    glm::glm
)
