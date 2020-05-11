function(set_target_lib_suffix target)
	if(WIN32)
		set(LIB_SUFFIX "_mp_")
	elseif(UNIX)
		set(LIB_SUFFIX ".mp.")
	else()
		message(FATAL_ERROR "unsupported system ${CMAKE_SYSTEM_NAME}")
	endif()
	if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "(x86|X86|i686|i386)$")
		if(WIN32)
			set(LIB_ARCH "x86")
		else()
			set(LIB_ARCH "i386")
		endif()
	elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "(amd64|AMD64|x86_64)$")
		if(WIN32)
			set(LIB_ARCH "x64")
		else()
			set(LIB_ARCH "x86_64")
		endif()
	else()
		message(FATAL_ERROR "unsupported architecture ${CMAKE_SYSTEM_PROCESSOR}")
	endif()

	get_target_property(LIB_NAME ${target} OUTPUT_NAME)
	if (NOT LIB_NAME)
		set(LIB_NAME "${target}")
	endif() 

	set_target_properties(${target} PROPERTIES OUTPUT_NAME "${LIB_NAME}${LIB_SUFFIX}${LIB_ARCH}" PREFIX "")
endfunction()
