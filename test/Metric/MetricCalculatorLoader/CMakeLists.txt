cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

PROJECT( mloader)

set( WITH_FACETOOLS TRUE)
include( "$ENV{DEV_PARENT_DIR}/libbuild/cmake/FindLibs.cmake")

set( SRC_FILES ${PROJECT_SOURCE_DIR}/main)

add_executable( ${PROJECT_NAME} ${SRC_FILES})

set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
# If Windows, separate binaries from build data because need to copy in 3rd party dlls
include( "$ENV{DEV_PARENT_DIR}/libbuild/cmake/ExeInstall.cmake")

