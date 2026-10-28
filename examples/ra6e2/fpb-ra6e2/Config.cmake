# Configuration file for custom user settings

if(RENESAS_IDE AND RENESAS_IDE STREQUAL "e2studio")
    set(RASC_EXE_PATH "echo")
    message(NOTICE "RENESAS_IDE is defined, so RASC_EXE_PATH set to echo for information purposes")
elseif(DEFINED RASC_EXE_PATH)
    message("Using RASC_EXE_PATH defined via CLI -D: ${RASC_EXE_PATH}")
elseif(DEFINED ENV{RASC_EXE_PATH})
    set(RASC_EXE_PATH $ENV{RASC_EXE_PATH})
    message("Using RASC_EXE_PATH defined in environment: ${RASC_EXE_PATH}")
else()
    # No path given: auto-detect a Renesas Smart Configurator (RASC) install
    # under the common per-OS install roots, newest FSP version first. This
    # keeps the project buildable for any user/workspace without editing this
    # file -- set RASC_EXE_PATH (CLI or env) to override.
    file(GLOB _rasc_candidates
        "$ENV{HOME}/.renesas/platform/sc/ra/fsp_*/eclipse/rasc"
        "$ENV{LOCALAPPDATA}/Renesas/RA/sc_v*/eclipse/rasc.exe"
        "$ENV{LOCALAPPDATA}/Programs/Renesas/RA/sc_v*/eclipse/rasc.exe"
        "C:/Renesas/RA/sc_v*/eclipse/rasc.exe")
    if(_rasc_candidates)
        list(SORT _rasc_candidates)
        list(GET _rasc_candidates -1 RASC_EXE_PATH)
        message("Auto-detected RASC_EXE_PATH: ${RASC_EXE_PATH}")
    else()
        message(FATAL_ERROR "Could not auto-detect a Renesas Smart Configurator (RASC) install. "
            "Set RASC_EXE_PATH via -DRASC_EXE_PATH=... or the RASC_EXE_PATH environment variable.")
    endif()
endif()
