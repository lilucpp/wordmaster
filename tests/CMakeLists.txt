# Google Test

# auto pull gtest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# 测试辅助库（共享代码）
add_library(test_common STATIC
    test_helpers.cpp
)

target_link_libraries(test_common
    Qt5::Core
    Qt5::Sql
    gtest        # 改这里
    gtest_main   # 改这里
)

if(UNIX)
    target_link_libraries(test_common
        pthread
    )
endif()


# 单元测试
set(UNIT_TESTS
    unit/test_sqlite_adapter
    unit/test_book_repository
    unit/test_word_repository
    unit/test_sm2_algorithm
)

foreach(test ${UNIT_TESTS})
    get_filename_component(test_name ${test} NAME)
    add_executable(${test_name}
        ${test}.cpp
        ${DOMAIN_SOURCES}
        ${INFRASTRUCTURE_SOURCES}
        ${APPLICATION_SOURCES}
    )
    
    target_link_libraries(${test_name}
        test_common
        Qt5::Core
        Qt5::Widgets
        Qt5::Sql
        gtest        # 改这里
        gtest_main   # 改这里
    )
    
if(UNIX)
    target_link_libraries(${test_name}
        pthread
    )
endif()
    
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# 集成测试
set(INTEGRATION_TESTS
    integration/test_book_import
    integration/test_study_flow
)

foreach(test ${INTEGRATION_TESTS})
    get_filename_component(test_name ${test} NAME)
    add_executable(${test_name}
        ${test}.cpp
        ${DOMAIN_SOURCES}
        ${INFRASTRUCTURE_SOURCES}
        ${APPLICATION_SOURCES}
    )
    
    target_link_libraries(${test_name}
        test_common
        Qt5::Core
        Qt5::Widgets
        Qt5::Sql
        gtest        # 改这里
        gtest_main   # 改这里
    )
if(UNIX)
    target_link_libraries(${test_name}
        pthread
    )
endif()
    
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
