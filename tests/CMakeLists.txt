# Backend Tests
set(BACKEND_TESTS
    test_PathUtils
    test_RegMapTreeItem
    test_RegMapTreeModel
    test_Serialization
    test_CodeGenerator
    test_Formats
    test_ThemeManager
)

foreach(test_name ${BACKEND_TESTS})
    qt_add_executable(${test_name} backend/${test_name}.cpp)
    target_compile_options(${test_name} PRIVATE -Wfatal-errors)
    target_link_libraries(${test_name} PRIVATE rmap_core Qt6::Test)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
    set_property(TARGET ${test_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endforeach()

# Frontend Tests
set(FRONTEND_TESTS
    test_RegMapWindow
    test_RegConfigWindow
    test_PreferencesWindow
    test_Delegates
)

foreach(test_name ${FRONTEND_TESTS})
    qt_add_executable(${test_name} frontend/${test_name}.cpp)
    target_compile_options(${test_name} PRIVATE -Wfatal-errors)
    target_link_libraries(${test_name} PRIVATE rmap_core Qt6::Test)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
    set_property(TARGET ${test_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endforeach()
