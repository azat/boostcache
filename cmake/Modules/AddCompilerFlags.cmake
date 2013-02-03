
include(AddCompilerFlag)

macro(AddCompilerFlags _flags_string)
    string(REPLACE " " ";" _flags ${_flags_string})
    foreach(_flag ${_flags})
        AddCompilerFlag("${_flag}" _good)
        if(NOT _good)
            message(FATAL_ERROR "${_flag} not supported. Try to update compiler, or install g++.")
        endif(NOT _good)
    endforeach(_flag)
endmacro(AddCompilerFlags)
