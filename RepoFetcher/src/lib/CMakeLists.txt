# CMakeList.txt: projeto de CMake para QtSample, incluir origem e definir
# lógica específica a um projeto aqui.
#

# Adicione a origem ao executável deste projeto.
cmake_minimum_required(VERSION 3.22)

macro(add_version_file OUTPUT_FILE)
	file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cxx.in SOURCE_CONTENTS)
	string(REGEX REPLACE SR2_MAJOR "${PROJECT_VERSION_MAJOR}" OUTPUT_SOURCE ${SOURCE_CONTENTS})
	string(REGEX REPLACE SR2_MINOR "${PROJECT_VERSION_MINOR}" OUTPUT_SOURCE ${OUTPUT_SOURCE})
	string(REGEX REPLACE SR2_PATCH "${PROJECT_VERSION_PATCH}" OUTPUT_SOURCE ${OUTPUT_SOURCE})
	file(WRITE ${OUTPUT_FILE} ${OUTPUT_SOURCE})
endmacro()

set(TARGET_LIB_NAME sr2lib)
set(TARGET_NAME sr2)

add_version_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp)
file(GLOB_RECURSE SR2_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")
file(GLOB_RECURSE SR2_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
file(GLOB_RECURSE SR2_LIB_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "components/*.hpp")
file(GLOB_RECURSE SR2_LIB_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "components/*.cpp")

trace_dependency(NAME zstd INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/zstd.py" LIMIT_SEARCH_PATHS)
trace_dependency(NAME zlib INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/zlib_ng.py" LIMIT_SEARCH_PATHS)
trace_library(NAME brotlienc INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/brotli.py" LIMIT_SEARCH_PATHS)
trace_library(NAME brotlidec INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/brotli.py" LIMIT_SEARCH_PATHS)
trace_library(NAME brotlicommon INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/brotli.py" LIMIT_SEARCH_PATHS)
trace_dependency(NAME lz4 INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/lz4.py" LIMIT_SEARCH_PATHS)
trace_dependency(NAME liblzma INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/lzma.py" LIMIT_SEARCH_PATHS)
trace_dependency(NAME OpenSSL INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/openssl.py" LIMIT_SEARCH_PATHS EXTRA_SUFFIXES "lib64/cmake/OpenSSL" "lib/cmake/OpenSSL" "share/cmake/OpenSSL")
trace_dependency(NAME jsoncpp INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/jsoncpp.py" LIMIT_SEARCH_PATHS)
trace_dependency(NAME nghttp2 INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/nghttp2.py" LIMIT_SEARCH_PATHS)
trace_dependency(NAME curl INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/libcurl.py" LIMIT_SEARCH_PATHS)
trace_dependency(NAME expat INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/expat.py" LIMIT_SEARCH_PATHS)
trace_library(NAME archive INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/libarchive.py" LIMIT_SEARCH_PATHS)

add_library(${TARGET_LIB_NAME} STATIC ${SR2_LIB_HDRS} ${SR2_LIB_SRCS})
target_include_directories(${TARGET_LIB_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/components>
	$<BUILD_INTERFACE:${CMAKE_PREFIX_PATH}/include>
)
target_link_libraries(${TARGET_LIB_NAME} PUBLIC sqlite3 CURL::libcurl)
set_cxx_project_standards(${TARGET_LIB_NAME} 20 FALSE)

add_executable(${TARGET_NAME} ${SR2_HDRS} ${SR2_SRCS})
target_link_libraries(${TARGET_NAME} PRIVATE ${TARGET_LIB_NAME})
target_include_directories(${TARGET_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

set_cxx_project_standards(${TARGET_NAME} 20 FALSE)