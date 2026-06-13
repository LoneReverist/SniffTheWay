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

find_package(glm CONFIG REQUIRED)
#find_package(stb CONFIG REQUIRED) # not needed, stb headers are globally available when vcpkg toolchain is set
find_package(nlohmann_json CONFIG REQUIRED)
set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

set(BUILD_VULKAN ON)
set(BUILD_OPENGL OFF)
set(BUILD_DEMOS OFF)
add_subdirectory(DreamhearthEngine)

add_executable(SniffTheWay)

set_target_properties(SniffTheWay PROPERTIES CXX_SCAN_FOR_MODULES ON)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders)
file(GLOB SHADER_FILES "${SHADER_SOURCE_DIR}/*")

if(BUILD_VULKAN)
    if(WIN32)
        set(GLSLANG_VALIDATOR_EXECUTABLE ${CMAKE_SOURCE_DIR}/buildtools/glslangValidator.exe)
    elseif(UNIX)
        set(GLSLANG_VALIDATOR_EXECUTABLE glslangValidator)
    endif()

    set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)
    file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

    foreach(SHADER ${SHADER_FILES})
        get_filename_component(FILE_NAME ${SHADER} NAME)
        set(OUTPUT_FILE ${SHADER_BINARY_DIR}/${FILE_NAME}.spv)

        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${GLSLANG_VALIDATOR_EXECUTABLE} -V ${SHADER} -o ${OUTPUT_FILE} -DBUILD_VULKAN
            DEPENDS ${SHADER}
            COMMENT "Compiling ${FILE_NAME} to SPIR-V"
            VERBATIM
        )

        list(APPEND COMPILED_SHADERS ${OUTPUT_FILE})
    endforeach()

    add_custom_target(VulkanShaders ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:SniffTheWay>/resources/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${SHADER_BINARY_DIR}" "$<TARGET_FILE_DIR:SniffTheWay>/resources/shaders"
        DEPENDS ${COMPILED_SHADERS}
        COMMENT "Copying Vulkan shaders to resources directory"
    )
    add_dependencies(SniffTheWay VulkanShaders)
elseif(BUILD_OPENGL)
    add_custom_target(OpenGLShaders ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:SniffTheWay>/resources/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${SHADER_SOURCE_DIR}" "$<TARGET_FILE_DIR:SniffTheWay>/resources/shaders"
        DEPENDS ${SHADER_FILES}
        COMMENT "Copying OpenGL shaders to resources directory"
    )
    add_dependencies(SniffTheWay OpenGLShaders)
endif()

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE MODULE_FILES "src/*.ixx")

target_sources(SniffTheWay PRIVATE
    FILE_SET CXX_MODULES TYPE CXX_MODULES
    BASE_DIRS src
    FILES ${MODULE_FILES}
    PRIVATE
        ${SOURCE_FILES}
        ${SHADER_FILES}
)
source_group("SniffTheWay/Shaders" FILES ${SHADER_FILES})

target_link_libraries(SniffTheWay PRIVATE
    glm::glm
    nlohmann_json::nlohmann_json
)

target_compile_definitions(SniffTheWay PRIVATE
    $<$<CONFIG:Debug>:SNIFF_THE_WAY_DEV_RESOURCES_PATH="${CMAKE_SOURCE_DIR}/resources">
)

if(BUILD_VULKAN)
    target_link_libraries(SniffTheWay PRIVATE
        VulkanRenderer
        GlfwVkWindow
    )
elseif(BUILD_OPENGL)
    target_link_libraries(SniffTheWay PRIVATE
        OpenGLRenderer
        GlfwGLWindow
    )
endif()

add_custom_command(TARGET SniffTheWay POST_BUILD
    COMMAND "$<$<CONFIG:Release>:${CMAKE_COMMAND};-E;copy_directory;${CMAKE_SOURCE_DIR}/resources;$<TARGET_FILE_DIR:SniffTheWay>/resources>"
    COMMAND_EXPAND_LISTS
)
