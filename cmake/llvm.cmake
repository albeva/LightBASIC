function(configure_llvm project_name)
    find_package(LLVM REQUIRED CONFIG)
    llvm_map_components_to_libnames(
        llvm_libs
        core
        support
        bitwriter
        transformUtils
    )
    target_include_directories(${project_name} SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
    target_link_libraries(${project_name} PRIVATE ${llvm_libs})
endfunction()