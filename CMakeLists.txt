# ============================================================================
# 项目基本信息
# ============================================================================
cmake_minimum_required(VERSION 3.16)
project(Pelr)

# ====================== 全局配置开关 ======================
set(AUTO_UPDATE_TRANSLATIONS ON)  # ON: 每次构建自动跑 lupdate 更新 .ts

# 单配置生成器必须给出构建类型。留空会让子工程拿到空值 ——
# FluentUIStyle ExternalProject 经 $<CONFIG> 透传该值（见文件末尾）。
# 保留原默认 Debug，因此不带 -DCMAKE_BUILD_TYPE 的旧命令行仍然可用。
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "CMAKE_BUILD_TYPE not set, defaulting to Debug")
endif()
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

# 调试特性总开关：控制台子系统(EXE_WIN32/WIN32_EXECUTABLE) + CONSOLE 宏 + 日志级别。
# 默认由构建类型推导，可用 -DDEBUG_MODE=ON/OFF 单独覆盖
# （例如在 Release 包上临时开详细日志，抓用户 bug）。
if(NOT DEFINED DEBUG_MODE)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(DEBUG_MODE OFF)
    else()
        set(DEBUG_MODE ON)
    endif()
endif()
message(STATUS "DEBUG_MODE = ${DEBUG_MODE}")
if(DEBUG_MODE)
    add_compile_definitions(CONSOLE)                # 启用控制台输出
endif()

# 修复控制台乱码
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# 平台与标准设置
if (WIN32)
    add_compile_definitions(UNICODE _UNICODE)   # Windows Unicode 支持
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)        # 自动处理 Qt MOC
set(CMAKE_AUTORCC ON)        # 自动处理 Qt RCC
set(CMAKE_AUTOUIC ON)        # 自动处理 Qt UIC

# Qt 路径与模块查找
# set(CMAKE_PREFIX_PATH "D:/Qt/6.10.1/mingw_64") # D:/Qt/6.10.1/mingw_64
find_package(Qt6 REQUIRED COMPONENTS
        Core Gui Widgets Network OpenGL OpenGLWidgets Svg
        WebChannel WebSockets Multimedia Concurrent MultimediaWidgets
        LinguistTools
)

# ============================================================================
# 资源与翻译文件
# ============================================================================

# 资源文件（.qrc）
qt_add_resources(QT_RESOURCES Resource.qrc)

# 翻译文件（.ts）
set(TRANSLATION_FILES
        translations/language_zh_CN.ts
        translations/language_en_US.ts
)

# ============================================================================
# 源文件收集（使用 GLOB，注意新文件需手动重新运行 CMake）
# ============================================================================

# 设置 Live2D 框架渲染方式（必须）
set(FRAMEWORK_SOURCE OpenGL)

# Live2D SDK 源文件（ LApp 封装层）
file(GLOB LAPP_2D_SRC "thirdParty/CubismNativeSamples/Samples/OpenGL/Demo/proj.win.cmake/src/*.cpp" "thirdParty/CubismNativeSamples/Samples/Common/*.cpp")

# 各功能模块源文件
file(GLOB AI_SRC "src/ai/*.cpp" "src/ai/*.h" "src/ai/*.hpp")
file(GLOB COMPATLAPP_SRC "src/compatLApp/*.cpp" "src/compatLApp/*.h" "src/compatLApp/*.hpp")
file(GLOB CORE_SRC "src/core/*.cpp" "src/core/*.h" "src/core/*.hpp")
file(GLOB KEYBOARD_SRC "src/keyboard/*.cpp" "src/keyboard/*.h" "src/keyboard/*.hpp")
file(GLOB MODEL_SRC "src/model/*.cpp" "src/model/*.h" "src/model/*.hpp")
file(GLOB TRANSLATION_SRC "src/translation/*.cpp" "src/translation/*.h" "src/translation/*.hpp")
file(GLOB TTS_SRC "src/tts/*.cpp" "src/tts/*.h" "src/tts/*.hpp")
file(GLOB UTILS_SRC "src/utils/*.cpp" "src/utils/*.h" "src/utils/*.hpp")
file(GLOB UI_SRC "src/ui/*.cpp" "src/ui/*.h" "src/ui/*.hpp")
file(GLOB UI_FILES "src/ui/*.ui")                     # .ui 文件单独收集
file(GLOB LIVE2D_SRC "src/live2d/*.cpp" "src/live2d/*.h" "src/live2d/*.hpp")

# 合并所有源文件（.cpp），供编译使用
set(ALL_CPP_SOURCES
        ${LAPP_2D_SRC}
        ${AI_SRC} ${COMPATLAPP_SRC} ${CORE_SRC} ${KEYBOARD_SRC} ${MODEL_SRC}
        ${TRANSLATION_SRC} ${TTS_SRC} ${UI_SRC} ${UTILS_SRC}
        ${LIVE2D_SRC}
        "src/main.cpp"
)

# 合并所有头文件（.h/.hpp），供 MOC 处理
set(ALL_HEADER_FILES
        ${AI_SRC} ${COMPATLAPP_SRC} ${CORE_SRC} ${KEYBOARD_SRC} ${MODEL_SRC}
        ${TRANSLATION_SRC} ${TTS_SRC} ${UI_SRC} ${UTILS_SRC} ${LIVE2D_SRC}
)

# 注意：${ALL_CPP_SOURCES} 只包含 .cpp，${ALL_HEADER_FILES} 包含 .h/.hpp
# 后续通过 qt5_wrap_cpp 从 ALL_HEADER_FILES 生成 MOC 文件

# ============================================================================
# Qt 自动化工具处理
# ============================================================================

# 生成翻译文件（.qm）
set(QM_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt_add_lupdate(${PROJECT_NAME}
    TS_FILES ${TRANSLATION_FILES}
    SOURCES ${ALL_CPP_SOURCES} ${ALL_HEADER_FILES} ${UI_FILES}
)
qt_add_translation(QM_FILES ${TRANSLATION_FILES})



# ============================================================================
# 第三方库：kissfft
# ============================================================================
add_library(kissfft STATIC
    thirdParty/kissfft/kiss_fft.c
    thirdParty/kissfft/kiss_fftr.c
)
target_include_directories(kissfft PUBLIC thirdParty/kissfft)
set_target_properties(kissfft PROPERTIES AUTOMOC OFF)

# ============================================================================
# 第三方库：miniaudio (轻量音频解码，TTS 口形同步)
# ============================================================================
add_library(miniaudio STATIC
    thirdParty/miniaudio/miniaudio.c
)
target_include_directories(miniaudio PUBLIC thirdParty/miniaudio)
set_target_properties(miniaudio PROPERTIES AUTOMOC OFF)

# ============================================================================
# 第三方库：miniz (ZIP 解包，用于 Live2D 可选组件投放)
# 独立 target，不 add_subdirectory(miniz)，避免其 CMakeLists 污染全局 cache
# ============================================================================
add_library(miniz STATIC
    thirdParty/miniz/miniz.c
    thirdParty/miniz/miniz_tdef.c
    thirdParty/miniz/miniz_tinfl.c
    thirdParty/miniz/miniz_zip.c
)
# miniz_export.h 是 CMake 生成文件，submodule 中不含该文件。
# miniz 是静态库，导出宏恒为空 —— 直接生成最简头文件，不依赖 GenerateExportHeader 的语义。
set(MINIZ_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/miniz_include")
file(MAKE_DIRECTORY "${MINIZ_INCLUDE_DIR}")
file(WRITE "${MINIZ_INCLUDE_DIR}/miniz_export.h"
     "#ifndef MINIZ_EXPORT\n"
     "#define MINIZ_EXPORT\n"
     "#endif\n"
     "#ifndef MINIZ_NO_EXPORT\n"
     "#define MINIZ_NO_EXPORT\n"
     "#endif\n"
)
target_include_directories(miniz PUBLIC
    ${CMAKE_SOURCE_DIR}/thirdParty/miniz
    ${MINIZ_INCLUDE_DIR}
)
set_target_properties(miniz PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

# ============================================================================
# 第三方库：GLEW (从源码编译)
# ============================================================================
add_library(glew STATIC
    thirdParty/glew/src/glew.c
)
target_include_directories(glew PUBLIC
    thirdParty/glew/include
)
target_compile_definitions(glew PRIVATE GLEW_STATIC)
set_target_properties(glew PROPERTIES AUTOMOC OFF)

# ============================================================================
# 第三方库：GLFW (从源码编译)
# ============================================================================
file(GLOB GLFW_SRC
    "thirdParty/glfw/src/*.c"
)
# 排除非 Windows 的源文件，避免额外依赖
list(FILTER GLFW_SRC EXCLUDE REGEX ".*wayland.*|.*x11.*|.*wl_.*")

add_library(glfw STATIC ${GLFW_SRC})
target_include_directories(glfw PUBLIC
    thirdParty/glfw/include
)
target_compile_definitions(glfw PRIVATE _GLFW_WIN32)
set_target_properties(glfw PROPERTIES AUTOMOC OFF)

# 收集 Framework 源码，只保留 OpenGL 和公共代码
file(GLOB_RECURSE CUBISM_FRAMEWORK_SRC_TEMP
    "thirdParty/CubismNativeFramework/src/*.cpp"
)
# 排除 Direct3D 后端
list(FILTER CUBISM_FRAMEWORK_SRC_TEMP EXCLUDE REGEX ".*D3D[0-9]+.*\\.cpp")
# 排除 Vulkan 后端
list(FILTER CUBISM_FRAMEWORK_SRC_TEMP EXCLUDE REGEX ".*Vulkan.*\\.cpp")

# 将 Framework 编译为静态库
add_library(Framework STATIC ${CUBISM_FRAMEWORK_SRC_TEMP})
set_target_properties(Framework PROPERTIES AUTOMOC OFF)

# 头文件路径
target_include_directories(Framework PUBLIC
    thirdParty/Core/include
    thirdParty/CubismNativeFramework/src
    thirdParty/glew/include
    thirdParty/glfw/include
)

# 使每个编译单元自动包含 glew.h，解决 GL 类型未定义
target_compile_options(Framework PRIVATE -include GL/glew.h)

# 定义 GLEW 静态链接（编译的是静态 GLEW）
target_compile_definitions(Framework PRIVATE GLEW_STATIC)

# 链接依赖
target_link_libraries(Framework PUBLIC
    glew
    glfw
    opengl32
    glu32
)

# ============================================================================
# 头文件包含路径（全局）
# ============================================================================
include_directories(
        ./
        src/ai
        src/core
        src/keyboard
        src/model
        src/translation
        src/tts
        src/ui
        src/utils
        src/live2d
        ${PROJECT_SOURCE_DIR}/thirdParty/CubismNativeFramework/src
        ${PROJECT_SOURCE_DIR}/thirdParty/Core/include
        ${PROJECT_SOURCE_DIR}/thirdParty/glew/include
        ${PROJECT_SOURCE_DIR}/thirdParty/glew/include/GL
        ${PROJECT_SOURCE_DIR}/thirdParty/glfw/include
        ${PROJECT_SOURCE_DIR}/thirdPartyglfw/include/GLFW
        ${PROJECT_SOURCE_DIR}/thirdParty/stb
        # ${PROJECT_SOURCE_DIR}/thirdParty/LAppLive2D/inc
        ${PROJECT_SOURCE_DIR}/thirdParty/CubismNativeSamples/Samples/Common
        ${PROJECT_SOURCE_DIR}/thirdParty/CubismNativeSamples/Samples/OpenGL/Demo/proj.win.cmake/src
        ${PROJECT_SOURCE_DIR}/thirdParty/kissfft
        ${CMAKE_CURRENT_BINARY_DIR}      # 存放生成的 ui_*.h 文件
)

# 指定 UI 文件搜索路径（AUTOUIC 需要）
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui")

# ===== 兼容 SDK 补丁 =====
# 优先搜索包装头文件（修复 OpenGL 类型缺失）
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/compatSDK)
# 优先级最高的兼容层头文件（替代 Demo 类）
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/compatLApp)

# 排除 Demo 原版类实现
list(FILTER ALL_CPP_SOURCES EXCLUDE REGEX "thirdParty/CubismNativeSamples.*/LAppDelegate\\.cpp")
list(FILTER ALL_CPP_SOURCES EXCLUDE REGEX "thirdParty/CubismNativeSamples.*/LAppLive2DManager\\.cpp")
list(FILTER ALL_CPP_SOURCES EXCLUDE REGEX "thirdParty/CubismNativeSamples.*/LAppModel\\.cpp")
list(FILTER ALL_CPP_SOURCES EXCLUDE REGEX "thirdParty/CubismNativeSamples.*/LAppView\\.cpp")
list(FILTER ALL_CPP_SOURCES EXCLUDE REGEX "thirdParty/CubismNativeSamples.*/LAppPal\\.cpp")
list(FILTER ALL_CPP_SOURCES EXCLUDE REGEX "thirdParty/CubismNativeSamples.*/LAppDefine\\.cpp")
# 排除 Demo 中的入口文件（它们也有 main 函数）
list(FILTER ALL_CPP_SOURCES EXCLUDE REGEX "thirdParty/CubismNativeSamples.*/main\\.cpp")
list(FILTER ALL_CPP_SOURCES EXCLUDE REGEX "thirdParty/CubismNativeSamples.*/mainMinimum\\.cpp")

# ============================================================================
# 可执行文件
# ============================================================================

if(DEBUG_MODE)
    set(EXE_WIN32)
else()
    set(EXE_WIN32 WIN32)
endif()
add_compile_definitions(GLEW_STATIC)
add_executable(${PROJECT_NAME}
        ${EXE_WIN32}                    # WIN32 或空
        ${ALL_CPP_SOURCES}         # 所有 .cpp 源文件
        ${QT_RESOURCES}            # RCC 生成的资源文件
        ${QM_FILES}                # 翻译文件（.qm）
)

# ============================================================================
# 链接库
# ============================================================================
target_link_libraries(${PROJECT_NAME}
        Framework          # Cubism Framework 静态库
        kissfft
        miniaudio          # 音频解码 (WAV/MP3)
        miniz         # ZIP 解包 (Live2D 组件投放)
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        Qt6::Svg
        Qt6::WebChannel
        Qt6::WebSockets
        Qt6::Multimedia
        Qt6::Concurrent
        Qt6::MultimediaWidgets
        ksuser
        dbghelp
)
if(AUTO_UPDATE_TRANSLATIONS)
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_lupdate)
endif()
# 强制在 Qt OpenGL 头前包含 GLEW，避免符号冲突
target_compile_options(${PROJECT_NAME} PRIVATE -include GL/glew.h)

# ============================================================================
# voicevox 插件（唯一接触 voicevox_core 的目标）
# voicevox_core.h 的函数全部带 __declspec(dllimport)，主 exe 无法用 QLibrary
# 自行绑定，因此必须隔离到独立 DLL；主 exe 不再链接 voicevox。
# ============================================================================
file(GLOB VOICEVOX_PLUGIN_SRC
        "src/plugins/voicevox/*.cpp"
        "src/plugins/voicevox/*.hpp")
add_library(local_voicevox MODULE ${VOICEVOX_PLUGIN_SRC})
set_target_properties(local_voicevox PROPERTIES
        PREFIX ""                                   # MinGW 默认加 lib 前缀，必须去掉
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins")
target_include_directories(local_voicevox PRIVATE
        "${PROJECT_SOURCE_DIR}/thirdParty/voicevox_core/c_api/include")
target_link_directories(local_voicevox PRIVATE
        "${CMAKE_SOURCE_DIR}/thirdParty/voicevox_core/c_api/lib")
target_link_libraries(local_voicevox PRIVATE
        voicevox_core
        Qt6::Core)

# 将 .qm 文件复制到 translations/ 子目录
file(MAKE_DIRECTORY ${QM_OUTPUT_DIR})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QM_FILES} ${QM_OUTPUT_DIR}
    COMMENT "Copying .qm files to ${QM_OUTPUT_DIR}"
)

# ============================================================================
# Windows 平台后处理
# ============================================================================
if (WIN32)
    # 程序图标
    set(APP_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/public/image/Pelr.ico")
    set(APP_RC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/icon.rc")
    if (EXISTS ${APP_ICON_FILE} AND EXISTS ${APP_RC_FILE})
        target_sources(${PROJECT_NAME} PRIVATE ${APP_RC_FILE})
        # 根据 CONSOLE 设置是否显示控制台
        if(DEBUG_MODE)
             set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE FALSE)
         else()
             set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
         endif()
    else ()
        message(WARNING "Program icon file or RC script is missing, default icon will be used!")
    endif ()

    # 使用 windeployqt 自动部署 Qt 运行时（DLL、插件、翻译）
    if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        find_program(WINDEPLOYQT windeployqt PATHS ${CMAKE_PREFIX_PATH}/bin NO_DEFAULT_PATH)
        if(WINDEPLOYQT)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${WINDEPLOYQT} --no-translations $<TARGET_FILE:${PROJECT_NAME}>
                COMMENT "Deploying Qt runtime libraries via windeployqt..."
            )
        else()
            message(WARNING "windeployqt not found, skipping Qt auto-deployment")
        endif()
    endif ()

    # ==================== 可选组件投放（仅 Debug）====================
    # Release 构建不投放第三方可选组件，程序以"组件缺失"状态启动。
    # 注：copy_directory_if_different 为纯增量、不会删除目标已有文件，
    #     因此本地已有的 voicevox_core/models (1.6GB) 不会被清掉。
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CORE_TP_DIR "${CMAKE_SOURCE_DIR}/thirdParty/Core")
        set(VOICEVOX_TP_DIR "${CMAKE_SOURCE_DIR}/thirdParty/voicevox_core")

        # 1) Live2D Cubism Core -> Live2D/（规范加载位置，不再落 exe 根）
        #    LICENSE.md 已被 git 跟踪、恒存在，故不加 EXISTS 守卫；
        #    Live2DCubismCore.dll 被 .gitignore 忽略，缺则跳过。
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Live2D"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CORE_TP_DIR}/LICENSE.md"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Live2D"
            COMMENT "Live2D Cubism Core LICENSE -> Live2D/ (debug only)"
        )
        if(EXISTS "${CORE_TP_DIR}/dll/windows/x86_64/Live2DCubismCore.dll")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CORE_TP_DIR}/dll/windows/x86_64/Live2DCubismCore.dll"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Live2D"
                COMMENT "Live2D Cubism Core -> Live2D/ (debug only)"
            )
        else()
            message(STATUS "Live2DCubismCore.dll not found under thirdParty/Core (skip)")
        endif()

        # 2) voicevox_core 官方目录结构 -> 程序根 voicevox_core/
        #    投放 c_api/onnxruntime/dict，命中 ComponentPaths 的推荐候选(rank1)。
        foreach(_vpart IN ITEMS c_api onnxruntime dict)
            if(EXISTS "${VOICEVOX_TP_DIR}/${_vpart}")
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                        "${VOICEVOX_TP_DIR}/${_vpart}"
                        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/voicevox_core/${_vpart}"
                    COMMENT "voicevox_core/${_vpart} -> voicevox_core/ (debug only)"
                )
            else()
                message(STATUS "voicevox_core/${_vpart} not found (skip)")
            endif()
        endforeach()
        message(STATUS "voicevox_core/models not auto-copied by design (1.6GB); copy it manually to <out>/voicevox_core/models to enable voicevox")
    else()
        message(STATUS "Release build: optional components (Live2D Cubism Core, voicevox_core) not deployed")
    endif()

    # 复制 assets 文件夹（如果存在）
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/assets"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
            COMMENT "Copying assets folder"
        )
    else()
        message(WARNING "Folder not found: ${CMAKE_SOURCE_DIR}/assets")
    endif()

    # Cubism OpenGL 运行时资源：从 submodule 直接复制到输出（无需 VS 构建 Demo）
    # 着色器为 GLSL 文本，构建期仅复制；示例模型只拷子目录，跳过根目录 Demo UI PNG
    set(CUBISM_FW_SHADERS "${CMAKE_SOURCE_DIR}/thirdParty/CubismNativeFramework/src/Rendering/OpenGL/Shaders/Standard")
    set(CUBISM_SAMPLE_SHADERS "${CMAKE_SOURCE_DIR}/thirdParty/CubismNativeSamples/Samples/OpenGL/Shaders/Standard")
    set(CUBISM_SAMPLE_RES "${CMAKE_SOURCE_DIR}/thirdParty/CubismNativeSamples/Samples/Resources")

    if(EXISTS "${CUBISM_FW_SHADERS}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CUBISM_FW_SHADERS}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/FrameworkShaders"
            COMMENT "Copying Cubism FrameworkShaders -> output"
        )
    else()
        message(WARNING "Cubism FrameworkShaders not found: ${CUBISM_FW_SHADERS}")
    endif()

    if(EXISTS "${CUBISM_SAMPLE_SHADERS}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CUBISM_SAMPLE_SHADERS}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SampleShaders"
            COMMENT "Copying Cubism SampleShaders -> output"
        )
    else()
        message(WARNING "Cubism SampleShaders not found: ${CUBISM_SAMPLE_SHADERS}")
    endif()

    if(EXISTS "${CUBISM_SAMPLE_RES}")
        file(GLOB CUBISM_MODEL_DIRS RELATIVE "${CUBISM_SAMPLE_RES}" "${CUBISM_SAMPLE_RES}/*")
        foreach(ITEM ${CUBISM_MODEL_DIRS})
            if(IS_DIRECTORY "${CUBISM_SAMPLE_RES}/${ITEM}")
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${CUBISM_SAMPLE_RES}/${ITEM}"
                        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources/${ITEM}"
                    COMMENT "Copying Cubism sample model Resources/${ITEM} -> output"
                )
            endif()
        endforeach()
    else()
        message(WARNING "Cubism sample Resources not found: ${CUBISM_SAMPLE_RES}")
    endif()
endif ()

# ============================================================================
# FluentUIStyle 插件（独立构建，不干涉主工程变量）
# 源码通过 git submodule 管理：thirdParty/FluentUIStyle
# ============================================================================
if(EXISTS "${CMAKE_SOURCE_DIR}/thirdParty/FluentUIStyle/CMakeLists.txt")
    include(ExternalProject)

    ExternalProject_Add(FluentUIStylePlugin
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdParty/FluentUIStyle"
        BINARY_DIR "${CMAKE_BINARY_DIR}/_deps/FluentUIStyle-build"
        CMAKE_GENERATOR "${CMAKE_GENERATOR}"
        CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
        CMAKE_GENERATOR_TOOLSET "${CMAKE_GENERATOR_TOOLSET}"
        CMAKE_CACHE_ARGS
            -DCMAKE_MAKE_PROGRAM:FILEPATH=${CMAKE_MAKE_PROGRAM}
            -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_PREFIX_PATH}
            -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
            -DBUILD_LIBRARY:BOOL=OFF
            -DBUILD_PLUGIN:BOOL=ON
            -DBUILD_EXAMPLE:BOOL=OFF
            -DBUILD_AUDIOMATIC_MINI:BOOL=OFF
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=$<CONFIG>
        BUILD_ALWAYS TRUE
        EXCLUDE_FROM_ALL TRUE
    )
    add_dependencies(${PROJECT_NAME} FluentUIStylePlugin)
else()
    message(WARNING "FluentUIStyle not found at thirdParty/FluentUIStyle. Skipping style plugin build.")
endif()