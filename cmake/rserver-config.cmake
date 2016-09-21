include("${CMAKE_CURRENT_LIST_DIR}/rserver-targets.cmake")

set( RSERVER_TARGET rserver )


# Compute the installation prefix relative to this file.
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)

set_target_properties(${RSERVER_TARGET} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${_IMPORT_PREFIX}/include )

# copy INTERFACE_* properties
foreach (_RSERVER_PROPERTY_NAME IN ITEMS
										COMPILE_DEFINITIONS
										COMPILE_FEATURES
										COMPILE_OPTIONS
										INCLUDE_DIRECTORIES
										LINK_LIBRARIES
										POSITION_INDEPENDENT_CODE
		)
	get_target_property (_RSERVER_PROPERTY_VALUE ${RSERVER_TARGET} INTERFACE_${_RSERVER_PROPERTY_NAME})
	if (_RSERVER_PROPERTY_VALUE)
		set_target_properties(rserver PROPERTIES INTERFACE_${_RSERVER_PROPERTY_NAME} "${_RSERVER_PROPERTY_VALUE}" )
	endif ()
endforeach ()


# Cleanup temporary variables.
set(_IMPORT_PREFIX)