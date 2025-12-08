# Exec2 target
function(run_bootstrap)

	get_filename_component(PARENT_DIR "${PROJECT_BINARY_DIR}" DIRECTORY)
	set(BOOTSTRAP_DIR "${PARENT_DIR}/${CMAKE_CURRENT_PRESET}-bootstrap")

	execute_process(COMMAND cmake "--preset" "${CMAKE_CURRENT_PRESET}" "-DFETCHER_BOOTSTRAP=ON" "-B" "${BOOTSTRAP_DIR}" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
	execute_process(COMMAND cmake "--build" "${BOOTSTRAP_DIR}" "--target" "install" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

	include_directories(${CMAKE_INSTALL_PREFIX}/bin)

endfunction()

run_bootstrap()

trace_dependency(NAME zlib INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/zlib.cmake.json" WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")
trace_dependency(NAME zstd INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/zstd.cmake.json" WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")
trace_dependency(NAME brotli INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/brotli.cmake.json" WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")
trace_dependency(NAME OpenSSL INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/openssl.custom.json" USE_VSTOOLS WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")
trace_dependency(NAME nghttp2 INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/nghttp2.cmake.json" USE_VSTOOLS WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")
trace_dependency(NAME curl INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/samples/libcurl.cmake.json" USE_VSTOOLS WORKING_DIR "${CMAKE_INSTALL_PREFIX}/bin")

file(GLOB_RECURSE EXEC_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")
file(GLOB_RECURSE EXEC_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

add_executable(FileFetcher ${EXEC_HDRS} ${EXEC_SRCS})