cmake_minimum_required(VERSION 3.16)
project(WordMaster VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 自动化工具
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# include CMakeLists.txt 所在的目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 查找依赖
find_package(Qt5 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Sql
)

# 源文件目录
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)

# 包含目录
include_directories(
    ${SRC_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# 收集源文件
file(GLOB_RECURSE DOMAIN_SOURCES 
    "${SRC_DIR}/domain/*.cpp"
)

file(GLOB_RECURSE INFRASTRUCTURE_SOURCES 
    "${SRC_DIR}/infrastructure/*.cpp"
)

file(GLOB_RECURSE APPLICATION_SOURCES 
    "${SRC_DIR}/application/*.cpp"
)

file(GLOB_RECURSE PRESENTATION_SOURCES 
    "${SRC_DIR}/presentation/*.cpp"
)

# 主程序
add_executable(${PROJECT_NAME}
    ${SRC_DIR}/main.cpp
    ${DOMAIN_SOURCES}
    ${INFRASTRUCTURE_SOURCES}
    ${APPLICATION_SOURCES}
    ${PRESENTATION_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Sql
)

# 测试支持
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 代码覆盖率支持
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# CLI 工具
add_executable(wordmaster_cli
    tools/wordmaster_cli.cpp
    ${DOMAIN_SOURCES}
    ${INFRASTRUCTURE_SOURCES}
    ${APPLICATION_SOURCES}
)

target_link_libraries(wordmaster_cli
    Qt5::Core
    Qt5::Sql
)

# 安装
install(TARGETS ${PROJECT_NAME} wordmaster_cli
    RUNTIME DESTINATION bin
)

# 资源文件
install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources
    DESTINATION share/${PROJECT_NAME}
)