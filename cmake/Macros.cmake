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


macro( get_library_suffix lsuffix)
    set( _dsuffix "")
    string( TOLOWER "${CMAKE_BUILD_TYPE}" _build_type)
    if( _build_type MATCHES "debug")
        set( _dsuffix "d")
    endif()

    if(UNIX)
        if(BUILD_USING_SHARED_LIBS)
            set( _lsuffix "${_dsuffix}.so")
        else()
            set( _lsuffix "${_dsuffix}.a")
        endif()
    elseif(WIN32)
        get_msvc_suffix( _msvcSuffix)
        set( _lsuffix "${_dsuffix}${_msvcSuffix}.lib")
    else()
        message( FATAL_ERROR "Platform not supported!")
    endif()

    set( ${lsuffix} ${_lsuffix})
endmacro( get_library_suffix)

