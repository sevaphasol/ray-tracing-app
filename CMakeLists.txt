cmake_minimum_required( VERSION 3.16 )
project( Zemax LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party )

file( GLOB_RECURSE REACTOR_SOURCES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/zemax/*.cpp
)

add_executable( zemax ${REACTOR_SOURCES} )
target_include_directories( zemax PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/nlohmann/single_include
)

file( GLOB_RECURSE MY_HUI_IMPL_SRC CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/custom-hui-impl/*.cpp
)
add_library( MyHuiImpl STATIC ${MY_HUI_IMPL_SRC} )
# target_include_directories( MyHuiImpl PUBLIC 3rd_party/dr4hui-backend/interface/include )
target_link_libraries( MyHuiImpl PUBLIC dr4 )
target_include_directories( MyHuiImpl PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/nlohmann/single_include
)

target_link_libraries( zemax PRIVATE dr4 )
target_link_libraries( zemax PRIVATE pp  )
target_link_libraries( zemax PRIVATE MyHuiImpl )

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")
target_compile_options(zemax PRIVATE -O3)
# install(TARGETS zemax DESTINATION build)
