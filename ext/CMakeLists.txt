set(EXT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(EXT_BUILD_DIR "${CMAKE_BINARY_DIR}/ext")
set(EXT_VSIX "${CMAKE_SOURCE_DIR}/bin/idp-gdb.vsix")

# List all files you need to copy and track
set(EXT_FILES
    "${EXT_SRC_DIR}/package.json"
    "${EXT_SRC_DIR}/LICENSE"
    # add more files here if needed
)

# This command will always copy everything in ext/ to the build dir when you build
add_custom_target(prepare_extension ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${EXT_SRC_DIR}" "${EXT_BUILD_DIR}"
    DEPENDS ${EXT_FILES}
    COMMENT "Copying extension sources to build/ext"
)

add_custom_command(
    OUTPUT "${EXT_VSIX}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/bin"
    COMMAND npx --yes @vscode/vsce package --no-dependencies --allow-missing-repository --out "${EXT_VSIX}"
    WORKING_DIRECTORY "${EXT_BUILD_DIR}"
    DEPENDS prepare_extension
    COMMENT "Packaging VS Code extension with @vscode/vsce"
)

add_custom_target(vscode_ext
    DEPENDS "${EXT_VSIX}"
    COMMENT "VS Code extension .vsix built in bin/"
)