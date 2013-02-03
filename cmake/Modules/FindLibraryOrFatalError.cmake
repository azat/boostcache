
# TODO: move to separate file
# TODO: syntax like for "find_library", e.g.  "VERSION_GREATER X"
function(FindLibraryOrFatalError libraryName)
    unset(LIBRARY CACHE)
    find_library(LIBRARY NAMES ${libraryName})

    if(LIBRARY)
        message(STATUS "Looking for ${libraryName} - found")
    else(LIBRARY)
        message(FATAL_ERROR "Looking for ${libraryName} - not found")
    endif(LIBRARY)

    # In global scope
    set(LIBS ${LIBS} ${LIBRARY} PARENT_SCOPE)
endfunction(FindLibraryOrFatalError)
