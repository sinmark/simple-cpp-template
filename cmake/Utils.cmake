function(verbose_message content)
    if(VERBOSE_OUTPUT)
        message(${MESSAGE_TYPE} ${content})
    endif()
endfunction()