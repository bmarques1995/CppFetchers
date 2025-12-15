function(run_bootstrap)
	execute_process(COMMAND cmake "--preset" ${CMAKE_CURRENT_PRESET} -B "./out/build/${CMAKE_CURRENT_PRESET}-bootstrap" "-DFETCHER_BOOTSTRAP=ON"  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
	execute_process(COMMAND cmake "--build" "./out/build/${CMAKE_CURRENT_PRESET}-bootstrap" "--target" "install" WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endfunction()

run_bootstrap()

file(GLOB_RECURSE REPO_EXEC_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/exec/*.cpp")
file(GLOB_RECURSE REPO_EXEC_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/exec/*.hpp")

trace_dependency(NAME zlib INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/zlib_static.cmake.json" WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")
trace_dependency(NAME zstd INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/zstd_static.cmake.json" WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")
trace_dependency(NAME brotli INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/brotli_static.cmake.json" WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")
trace_dependency(NAME OpenSSL INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/openssl_static.custom.json" WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")
trace_dependency(NAME nghttp2 INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/nghttp2_static.cmake.json" WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")
trace_dependency(NAME curl INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/libcurl_static.cmake.json" WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")

set(TARGET_NAME FileFetcher)
add_executable(${TARGET_NAME} ${REPO_EXEC_SRCS} ${REPO_EXEC_HDRS})
target_include_directories(${TARGET_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/exec>)
#crypt32 bcrypt advapi32 normaliz
target_link_libraries(${TARGET_NAME} PRIVATE RepoFetcherLib CURL::libcurl_static nghttp2::nghttp2_static ws2_32 crypt32 bcrypt advapi32 normaliz)
target_compile_definitions(${TARGET_NAME} PRIVATE "CURL_STATICLIB")

set_property(TARGET ${TARGET_NAME}  PROPERTY CXX_STANDARD 20)

install(TARGETS ${TARGET_NAME} DESTINATION bin)