set(LIBRARY_NAME Discord)

#Base files
file(GLOB Discord_Source ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
    source_group("Discord" FILES ${Discord_Source})

#Define base source library
set(LIBRARY_SRCS
    ${Discord_Source}
)

#Define ENABLE_DISCORD if need
if (BUILD_DISCORD)
  add_definitions(-DENABLE_DISCORD)
endif()


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ${CMAKE_SOURCE_DIR}/src/game
  ${CMAKE_SOURCE_DIR}/src/game/Globals
  ${CMAKE_SOURCE_DIR}/src/game/Entities
  ${CMAKE_SOURCE_DIR}/src/game/Server
  ${CMAKE_SOURCE_DIR}/src/game/World
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/shared/Auth
  ${CMAKE_SOURCE_DIR}/src/shared/Config
  ${CMAKE_SOURCE_DIR}/src/shared/Common
  ${CMAKE_SOURCE_DIR}/src/shared/Database
  ${CMAKE_SOURCE_DIR}/src/shared/DataStores
  ${CMAKE_SOURCE_DIR}/src/shared/Network
  ${CMAKE_SOURCE_DIR}/src/shared/Log
  ${CMAKE_SOURCE_DIR}/src/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/shared/Utilities
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_SOURCE_DIR}/src/framework/Platform
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/shared
  ${MYSQL_INCLUDE_DIR}
  ${ACE_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
)

add_library(${LIBRARY_NAME} STATIC ${LIBRARY_SRCS})
target_link_libraries(${LIBRARY_NAME} dpp shared detour)
