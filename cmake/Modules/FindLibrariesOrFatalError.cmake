
include(FindLibraryOrFatalError)

# TODO: write like AddCompilerFlags, check errors in one place.
macro(FindLibrariesOrFatalError _libraries_string)
    string(REPLACE " " ";" _libraries ${_libraries_string})
    foreach(_library ${_libraries})
        FindLibraryOrFatalError("${_library}" _good)
    endforeach(_library)
endmacro(FindLibrariesOrFatalError)
