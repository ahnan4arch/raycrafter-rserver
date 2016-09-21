CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

PROJECT ( rserver )
set( RSERVER_VERSION 0.1 )

set(CMAKE_DEBUG_POSTFIX "d")
INCLUDE_DIRECTORIES ( include )

ADD_LIBRARY ( rserver STATIC
  src/main.cpp
  src/RenderServer.cpp
  src/WebsocketServer.cpp
  src/jsmn/jsmn.cpp
  src/jsmn/jsmn_util.cpp
  )

#find_package(websocketpp REQUIRED)
#if (websocketpp_FOUND)
#  message("-- Found websocketpp version ${websocketpp_VERSION}: ${websocketpp_INCLUDE_DIRS} ${websocketpp_DIR}")
#else ()
#  message(FATAL_ERROR "websocketpp not found")
#endif ()
#target_link_library( rserver websocketpp )

# trigger cxx standard (c++11)
#target_compile_features(rserver PUBLIC cxx_constexpr)
set_property(TARGET rserver PROPERTY CXX_STANDARD 11)


# install target (the lib file) and register the target in export set ---
install( TARGETS rserver DESTINATION lib EXPORT rserver-targets )
# copy header files
install( DIRECTORY include DESTINATION . )


# location of cmake config files in binary tree
set(ConfigPackageLocation lib/cmake/rserver)

include(CMakePackageConfigHelpers)

# create version config file ---
write_basic_package_version_file( "${CMAKE_CURRENT_BINARY_DIR}/rserver/rserver-config-version.cmake" VERSION ${RSERVER_VERSION} COMPATIBILITY AnyNewerVersion )

# export targets (which have been defined during previous calls to install)
export(EXPORT rserver-targets FILE "${CMAKE_CURRENT_BINARY_DIR}/rserver/rserver-targets.cmake" NAMESPACE Upstream:: )


# install rserver-targets cmake file ---
install( EXPORT rserver-targets DESTINATION ${ConfigPackageLocation} )


# install version and config package file ---
install( FILES cmake/rserver-config.cmake "${CMAKE_CURRENT_BINARY_DIR}/rserver/rserver-config-version.cmake" DESTINATION ${ConfigPackageLocation} COMPONENT Devel )

# for the final config file you can also use configure_file to replace some variables and move it into place
# create final package config file (by copying from source-tree)
#configure_file(cmake/rserver-config.cmake "${CMAKE_CURRENT_BINARY_DIR}/rserver/rserver-config.cmake" COPYONLY )



