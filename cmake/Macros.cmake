# Sets the correct library suffix for the linker given the platform and whether we want a debug build or not.
# User should set CMAKE_BUILD_TYPE [debug/...] and BUILD_SHARED_LIBS [true/false] (or BUILD_USING_SHARED_LIBS) before calling.
# lsuffix set to one of [platform;build type]:
# .so   [UNIX;SHARED]          d.so  [UNIX;DEBUG,SHARED]
# .a    [UNIX;STATIC]          d.a   [UNIX;DEBUG,STATIC]
# .lib  [WIN32;STATIC/SHARED]  d.lib  [WIN32;DEBUG,STATIC/SHARED]


macro( get_msvc_suffix suffix)
    set( ${suffix} "")
    if(MSVC)
        if( MSVC70 OR MSVC71 )
            set(MSVC_PREFIX "vc70")
        elseif( MSVC80 )
            set(MSVC_PREFIX "vc80")
        elseif( MSVC90 )
            set(MSVC_PREFIX "vc90")
        elseif( MSVC10 )
            set(MSVC_PREFIX "vc100")
        elseif( MSVC11 )
            set(MSVC_PREFIX "vc110")
        elseif( MSVC12 )
            set(MSVC_PREFIX "vc120")
        elseif( MSVC14 )
            set(MSVC_PREFIX "vc140")
        else()
            set(MSVC_PREFIX "vc150")
        endif()
        set( ${suffix} "-${MSVC_PREFIX}")
    endif(MSVC)
endmacro( get_msvc_suffix)


macro( get_debug_suffix dsuff)
    set( ${dsuff} "")
    string( TOLOWER "${CMAKE_BUILD_TYPE}" _build_type)
    if( _build_type MATCHES "debug")
        set( ${dsuff} "d")
    endif()
endmacro( get_debug_suffix)


macro( get_shared_suffix lsuffix)
    get_debug_suffix( _dsuffix)
    if(UNIX)
        set( ${lsuffix} "${_dsuffix}.so")
    elseif(WIN32)
        get_msvc_suffix( _msvcSuffix)
        set( ${lsuffix} "${_dsuffix}${_msvcSuffix}.dll")
    endif()
endmacro( get_shared_suffix)


macro( get_static_suffix lsuffix)
    get_debug_suffix( _dsuffix)
    if(UNIX)
        set( ${lsuffix} "${_dsuffix}.a")
    elseif(WIN32)
        get_msvc_suffix( _msvcSuffix)
        set( ${lsuffix} "${_dsuffix}${_msvcSuffix}.lib")
    endif()
endmacro( get_static_suffix)


# For looking for libraries
macro( get_library_suffix lsuffix)
    if(UNIX)
        if(BUILD_USING_SHARED_LIBS)
            get_shared_suffix( _lsuffix)
        else()
            get_static_suffix( _lsuffix)
        endif()
    elseif(WIN32)
        get_static_suffix( _lsuffix)
    else()
        message( FATAL_ERROR "Platform not supported!")
    endif()
    set( ${lsuffix} ${_lsuffix})
endmacro( get_library_suffix)


macro( copy_over_dll libName)
    get_shared_suffix( sosuff)
    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD      # post build event
        COMMAND ${CMAKE_COMMAND} -E copy_if_different          # execs "cmake -R copy_if_different ..."
        "$ENV{INSTALL_PARENT_DIR}/${libName}/bin/${libName}${sosuff}"         # dll to copy
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)                    # out path
endmacro( copy_over_dll)
