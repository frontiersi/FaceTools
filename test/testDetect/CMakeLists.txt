cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)
 
PROJECT(tapp)

set(WITH_FACETOOLS TRUE)
include( $ENV{DEV_PARENT_DIR}/libbuild/cmake/FindLibs.cmake)
 
add_executable(${PROJECT_NAME} main.cxx)
 
include( $ENV{DEV_PARENT_DIR}/libbuild/cmake/LinkTargets.cmake)
