# ============================================================================
# 项目基本信息
# ============================================================================
cmake_minimum_required(VERSION 3.16)
project(Pelr)

# ====================== 全局配置开关 ======================
set(DEBUG_MODE ON)           # ON: Debug, OFF: Release

# 根据 DEBUG_MODE 设置构建类型和子系统
if(DEBUG_MODE)
    set(CMAKE_BUILD_TYPE Debug)
    add_compile_definitions(CONSOLE)                # 启用控制台输出
    # Debug 模式不安装自定义消息处理器，消息直接输出到调试器/控制台
else()
    set(CMAKE_BUILD_TYPE Release)
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
set(CMAKE_PREFIX_PATH "D:/Qt/6.10.1/mingw_64")
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

# Live2D SDK 源文件（你自己的 LApp 封装层）
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

# 合并所有源文件（.cpp），供编译使用
set(ALL_CPP_SOURCES
        ${LAPP_2D_SRC}
        ${AI_SRC} ${COMPATLAPP_SRC} ${CORE_SRC} ${KEYBOARD_SRC} ${MODEL_SRC}
        ${TRANSLATION_SRC} ${TTS_SRC} ${UI_SRC} ${UTILS_SRC}
        "src/main.cpp"
)

# 合并所有头文件（.h/.hpp），供 MOC 处理
set(ALL_HEADER_FILES
        ${AI_SRC} ${COMPATLAPP_SRC} ${CORE_SRC} ${KEYBOARD_SRC} ${MODEL_SRC}
        ${TRANSLATION_SRC} ${TTS_SRC} ${UI_SRC} ${UTILS_SRC}
)

# 注意：${ALL_CPP_SOURCES} 只包含 .cpp，${ALL_HEADER_FILES} 包含 .h/.hpp
# 后续通过 qt5_wrap_cpp 从 ALL_HEADER_FILES 生成 MOC 文件

# ============================================================================
# Qt 自动化工具处理
# ============================================================================

# 生成翻译文件（.qm）
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

# 定义 GLEW 静态链接（因为我们编译的是静态 GLEW）
target_compile_definitions(Framework PRIVATE GLEW_STATIC)
set(CORE_DLL_PATH "${CMAKE_SOURCE_DIR}/thirdParty/Core/dll/windows/x86_64/Live2DCubismCore.dll")

# 链接依赖
target_link_libraries(Framework PUBLIC
    glew
    glfw
    opengl32
    glu32
    ${CORE_DLL_PATH}
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
        ${PROJECT_SOURCE_DIR}/thirdParty/voicevox_core/c_api/include
        ${CMAKE_CURRENT_BINARY_DIR}      # 存放生成的 ui_*.h 文件
)

# 指定 UI 文件搜索路径（AUTOUIC 需要）
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui")
# ============================================================================
# 链接库路径
# ============================================================================
link_directories(
    "${CMAKE_SOURCE_DIR}/thirdParty/voicevox_core/c_api/lib"
)

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
        voicevox_core
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
)

# 强制在 Qt OpenGL 头前包含 GLEW，避免符号冲突
target_compile_options(${PROJECT_NAME} PRIVATE -include GL/glew.h)

# ============================================================================
# Windows 平台后处理
# ============================================================================
if (WIN32)
    # 程序图标
    set(APP_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/assets/image/Pelr.ico")
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

    # 复制翻译文件（.qm）到输出目录
    set(QM_OUTPUT_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/translations")
    foreach (qm_file ${QM_FILES})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${QM_OUTPUT_DIR}
                COMMAND ${CMAKE_COMMAND} -E copy ${qm_file} ${QM_OUTPUT_DIR}/
                COMMENT "Copying translation file: ${qm_file}")
    endforeach ()

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

    # 单独复制 voicevox_core.dll（如果没有放入 lib）
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/thirdParty/voicevox_core/c_api/lib/voicevox_core.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMENT "Copying voicevox_core DLL: voicevox_core.dll"

        )
    # voicevox_onnxruntime.dll 到 exe 同目录
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/thirdParty/voicevox_core/onnxruntime/lib/voicevox_onnxruntime.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying voicevox_core DLL: voicevox_onnxruntime.dll"
)
    # 复制 Live2DCubismCore.dll 到 exe 同目录
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/thirdParty/Core/dll/windows/x86_64/Live2DCubismCore.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying Live2DCubismCore DLL: Live2DCubismCore.dll"
)

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

    # 2. 自动复制 Resources 下的所有子文件夹，直接放到输出根目录
    set(RES_DIR "${CMAKE_SOURCE_DIR}/Resources")
    if(EXISTS "${RES_DIR}")
        file(GLOB RES_ITEMS RELATIVE "${RES_DIR}" "${RES_DIR}/*")
        foreach(ITEM ${RES_ITEMS})
            if(IS_DIRECTORY "${RES_DIR}/${ITEM}")
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${RES_DIR}/${ITEM}"
                        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${ITEM}"
                    COMMENT "Copying Resources/${ITEM} -> output directory"
                )
            endif()
        endforeach()
    else()
        message(WARNING "Resources directory not found: ${RES_DIR}")
    endif()
endif ()