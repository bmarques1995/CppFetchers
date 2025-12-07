# Exec2 target
function(run_bootstrap)

	get_filename_component(PARENT_DIR "${PROJECT_BINARY_DIR}" DIRECTORY)
	set(BOOTSTRAP_DIR "${PARENT_DIR}/${CMAKE_CURRENT_PRESET}-bootstrap")

	execute_process(COMMAND cmake "--preset" "${CMAKE_CURRENT_PRESET}" "-DFETCHER_BOOTSTRAP=ON" "-B" "${BOOTSTRAP_DIR}" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
	execute_process(COMMAND cmake "--build" "${BOOTSTRAP_DIR}" "--target" "install" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

	include_directories(${CMAKE_INSTALL_PREFIX}/bin)

endfunction()

run_bootstrap()
add_executable(FileFetcher main.cpp)