SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

cmake_minimum_required(VERSION 2.8)
project(ffengine)
add_definitions(-g -O2 -Wall -static)
add_definitions(-std=c++11)
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	MESSAGE(STATUS "current platform: Linux ")
    set(EXT_LIBS 
        dl mysqlclient
    )
ELSE ()
	MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

include_directories(${PROJECT_SOURCE_DIR}/fflib ${PROJECT_SOURCE_DIR}/lib3rd ${PROJECT_SOURCE_DIR}/src  
            /usr/include/mysql/ /usr/include/python2.7/
            ${EXT_INCLUDE}
)
link_directories(/usr/local/lib /usr/lib64/mysql ${PROJECT_SOURCE_DIR} ${EXT_LINK})

file(GLOB_RECURSE ENGINE_LIB_SRC "fflib/*.c*") 
foreach(filename ${ENGINE_LIB_SRC})  
message(${filename})
endforeach()

file(GLOB_RECURSE WORKER_CPP_SRC "worker/cpp/*.cpp") 
foreach(filename ${WORKER_CPP_SRC})  
message(${filename})
endforeach()

file(GLOB_RECURSE WORKER_PY_SRC "worker/py/*.cpp") 
foreach(filename ${WORKER_PY_SRC})  
message(${filename})
endforeach()

set(ENGINE_SRC_LIST 
    ${ENGINE_LIB_SRC}
    ${PROJECT_SOURCE_DIR}/engine/main.cpp
)
set(WORKER_CPP_SRC_LIST 
    ${ENGINE_LIB_SRC} ${WORKER_CPP_SRC}
)
set(WORKER_PY_SRC_LIST 
    ${ENGINE_LIB_SRC} ${WORKER_PY_SRC}
)

add_executable(ffengine ${ENGINE_SRC_LIST})
target_link_libraries(ffengine pthread ${EXT_LIBS} crypto ssl z curl)

if(PYVER)
    message("user python version:" ${PYVER})
else()
    message("user default python version:python2.7")
    set(PYVER "python2.7") 
endif()

add_executable(ffworkercpp ${WORKER_CPP_SRC_LIST})
target_link_libraries(ffworkercpp pthread ${EXT_LIBS} crypto ssl z curl)

add_executable(ffworkerpy ${WORKER_PY_SRC_LIST})
target_link_libraries(ffworkerpy pthread ${EXT_LIBS} crypto ssl z curl ${PYVER})
