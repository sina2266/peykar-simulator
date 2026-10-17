# =============================================================================
# Platform specific settings
# =============================================================================
if(WIN32 OR MINGW)
    set(IV_WINDOWS_BUILD TRUE)
endif()

# used in config.h
if(${CMAKE_SYSTEM_NAME} MATCHES "CYGWIN")
    set(CYGWIN 1)
endif()
