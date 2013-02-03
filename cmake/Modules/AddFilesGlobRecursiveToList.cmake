
# TODO: add all options for file() and list(APPEND )

macro(AddFilesGlobRecursiveToList _list_name _glob_pattern)
    unset(AddFilesGlobRecursiveToList_tmp CACHE)

    file(GLOB_RECURSE AddFilesGlobRecursiveToList_tmp ${_glob_pattern})
    list(APPEND ${_list_name} ${AddFilesGlobRecursiveToList_tmp})
endmacro(AddFilesGlobRecursiveToList)
