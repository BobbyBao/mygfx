


# ucm_dir_list
# Returns a list of subdirectories for a given directory
macro(ucm_dir_list thedir result)
    file(GLOB sub-dir "${thedir}/*")
    set(list_of_dirs "")
    foreach(dir ${sub-dir})
        if(IS_DIRECTORY ${dir})
            get_filename_component(DIRNAME ${dir} NAME)
            LIST(APPEND list_of_dirs ${DIRNAME})
        endif()
    endforeach()
    set(${result} ${list_of_dirs})
endmacro()

# ucm_trim_front_words
# Trims X times the front word from a string separated with "/" and removes
# the front "/" characters after that (used for filters for visual studio)
macro(ucm_trim_front_words source out num_filter_trims)
    set(result "${source}")
    set(counter 0)
    while(${counter} LESS ${num_filter_trims})
        MATH(EXPR counter "${counter} + 1")
        # removes everything at the front up to a "/" character
        string(REGEX REPLACE "^([^/]+)" "" result "${result}")
        # removes all consecutive "/" characters from the front
        string(REGEX REPLACE "^(/+)" "" result "${result}")
    endwhile()
    set(${out} ${result})
endmacro()

# ucm_remove_files
# Removes source files from a list of sources (path is the relative path for it to be found)
macro(ucm_remove_files)
    cmake_parse_arguments(ARG "" "FROM" "" ${ARGN})
    
    if("${ARG_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "Need to pass some relative files to ucm_remove_files()")
    endif()
    if(${ARG_FROM} STREQUAL "")
        message(FATAL_ERROR "Need to pass FROM and a variable name to ucm_remove_files()")
    endif()
    
    foreach(cur_file ${ARG_UNPARSED_ARGUMENTS})
        list(REMOVE_ITEM ${ARG_FROM} ${cur_file})
    endforeach()
endmacro()

# ucm_remove_directories
# Removes all source files from the given directories from the sources list
macro(ucm_remove_directories)
    cmake_parse_arguments(ARG "" "FROM" "MATCHES" ${ARGN})
    
    if("${ARG_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "Need to pass some relative directories to ucm_remove_directories()")
    endif()
    if(${ARG_FROM} STREQUAL "")
        message(FATAL_ERROR "Need to pass FROM and a variable name to ucm_remove_directories()")
    endif()
    
    foreach(cur_dir ${ARG_UNPARSED_ARGUMENTS})
        foreach(cur_file ${${ARG_FROM}})
            string(REGEX MATCH ${cur_dir} res ${cur_file})
            if(NOT "${res}" STREQUAL "")
                if("${ARG_MATCHES}" STREQUAL "")
                    list(REMOVE_ITEM ${ARG_FROM} ${cur_file})
                else()
                    foreach(curr_ptrn ${ARG_MATCHES})
                        string(REGEX MATCH ${curr_ptrn} res ${cur_file})
                        if(NOT "${res}" STREQUAL "")
                            list(REMOVE_ITEM ${ARG_FROM} ${cur_file})
                            break()
                        endif()
                    endforeach()
                endif()
            endif()
        endforeach()
    endforeach()
endmacro()

# ucm_add_files_impl
macro(ucm_add_files_impl result trim files)
    foreach(cur_file ${files})
        SET(${result} ${${result}} ${cur_file})
        get_filename_component(FILEPATH ${cur_file} PATH)
        ucm_trim_front_words("${FILEPATH}" FILEPATH "${trim}")
        # replacing forward slashes with back slashes so filters can be generated (back slash used in parsing...)
        STRING(REPLACE "/" "\\" FILTERS "${FILEPATH}")
        SOURCE_GROUP("${FILTERS}" FILES ${cur_file})
    endforeach()
endmacro()

# ucm_add_files
# Adds files to a list of sources
macro(ucm_add_files)
    cmake_parse_arguments(ARG "" "TO;FILTER_POP" "" ${ARGN})
    
    if("${ARG_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "Need to pass some relative files to ucm_add_files()")
    endif()
    if(${ARG_TO} STREQUAL "")
        message(FATAL_ERROR "Need to pass TO and a variable name to ucm_add_files()")
    endif()
    
    if("${ARG_FILTER_POP}" STREQUAL "")
        set(ARG_FILTER_POP 0)
    endif()
    
    ucm_add_files_impl(${ARG_TO} ${ARG_FILTER_POP} "${ARG_UNPARSED_ARGUMENTS}")
endmacro()


# ucm_add_dir_impl
macro(ucm_add_dir_impl result rec trim dirs_in additional_ext)
    set(dirs "${dirs_in}")
    
    # handle the "" and "." cases
    if("${dirs}" STREQUAL "" OR "${dirs}" STREQUAL ".")
        set(dirs "./")
    endif()
    
    foreach(cur_dir ${dirs})
        # to circumvent some linux/cmake/path issues - barely made it work...
        if(cur_dir STREQUAL "./")
            set(cur_dir "")
        else()
            set(cur_dir "${cur_dir}/")
        endif()
        
        # since unix is case sensitive - add these valid extensions too
        # we don't use "UNIX" but instead "CMAKE_HOST_UNIX" because we might be cross
        # compiling (for example emscripten) under windows and UNIX may be set to 1
        # Also OSX is case insensitive like windows...
        set(additional_file_extensions "")
        if(CMAKE_HOST_UNIX AND NOT APPLE)
            set(additional_file_extensions
                "${cur_dir}*.CPP"
                "${cur_dir}*.C"
                "${cur_dir}*.H"
                "${cur_dir}*.HPP"
                )
        endif()
        
        foreach(ext ${additional_ext})
            list(APPEND additional_file_extensions "${cur_dir}*.${ext}")
        endforeach()
        
        # find all sources and set them as result
        FILE(GLOB found_sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        # https://gcc.gnu.org/onlinedocs/gcc-4.4.1/gcc/Overall-Options.html#index-file-name-suffix-71
        # sources
            "${cur_dir}*.cpp"
            "${cur_dir}*.cxx"
            "${cur_dir}*.c++"
            "${cur_dir}*.cc"
            "${cur_dir}*.cp"
            "${cur_dir}*.c"
            "${cur_dir}*.i"
            "${cur_dir}*.ii"
        # headers
            "${cur_dir}*.h"
            "${cur_dir}*.h++"
            "${cur_dir}*.hpp"
            "${cur_dir}*.hxx"
            "${cur_dir}*.hh"
            "${cur_dir}*.inl"
            "${cur_dir}*.inc"
            "${cur_dir}*.ipp"
            "${cur_dir}*.ixx"
            "${cur_dir}*.txx"
            "${cur_dir}*.tpp"
            "${cur_dir}*.tcc"
            "${cur_dir}*.tpl"
            ${additional_file_extensions})
        SET(${result} ${${result}} ${found_sources})
        
        # set the proper filters
        ucm_trim_front_words("${cur_dir}" cur_dir "${trim}")
        # replacing forward slashes with back slashes so filters can be generated (back slash used in parsing...)
        STRING(REPLACE "/" "\\" FILTERS "${cur_dir}")
        SOURCE_GROUP("${FILTERS}" FILES ${found_sources})
    endforeach()
    
    if(${rec})
        foreach(cur_dir ${dirs})
            ucm_dir_list("${cur_dir}" subdirs)
            foreach(subdir ${subdirs})
                ucm_add_dir_impl(${result} ${rec} ${trim} "${cur_dir}/${subdir}" "${additional_ext}")
            endforeach()
        endforeach()
    endif()
endmacro()

# ucm_add_dirs
# Adds all files from directories traversing them recursively to a list of sources
# and generates filters according to their location (accepts relative paths only).
# Also this macro trims X times the front word from the filter string for visual studio filters.
macro(ucm_add_dirs)
    cmake_parse_arguments(ARG "RECURSIVE" "TO;FILTER_POP" "ADDITIONAL_EXT" ${ARGN})
    
    if(${ARG_TO} STREQUAL "")
        message(FATAL_ERROR "Need to pass TO and a variable name to ucm_add_dirs()")
    endif()
    
    if("${ARG_FILTER_POP}" STREQUAL "")
        set(ARG_FILTER_POP 0)
    endif()
    
    ucm_add_dir_impl(${ARG_TO} ${ARG_RECURSIVE} ${ARG_FILTER_POP} "${ARG_UNPARSED_ARGUMENTS}" "${ARG_ADDITIONAL_EXT}")
endmacro()