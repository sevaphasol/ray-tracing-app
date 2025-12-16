cmake_minimum_required( VERSION 3.16 )
project( rta LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

###############################################################################

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party )

###############################################################################
#
# file( GLOB_RECURSE HUI_BACKEND_SRC CONFIGURE_DEPENDS
	# ${CMAKE_CURRENT_SOURCE_DIR}/custom-hui/*.cpp
# )
# add_library( hui-backend STATIC ${HUI_BACKEND_SRC} )
# target_link_libraries( hui-backend PUBLIC dr4 )
# target_include_directories( hui-backend PUBLIC
	# ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/nlohmann/single_include
# )
#
###############################################################################

file( GLOB_RECURSE RTA_SOURCES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/rta/*.cpp
)
add_executable( rta ${RTA_SOURCES} )
target_include_directories( rta PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/nlohmann/single_include
)
target_link_libraries( rta PRIVATE dr4 )
target_link_libraries( rta PRIVATE cum )
target_link_libraries( rta PRIVATE pp  )

target_link_libraries( rta PRIVATE custom-hui-interface )
target_link_libraries( rta PRIVATE custom-hui )

set( CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}" )
target_compile_options( rta PRIVATE -O3 -march=native -mtune=native -DNDEBUG )
