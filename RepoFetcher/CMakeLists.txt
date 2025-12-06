# CMakeList.txt: projeto de CMake para RepoFetcher, incluir origem e definir
# lógica específica a um projeto aqui.
#

# Adicione a origem ao executável deste projeto.

file(GLOB_RECURSE REPO_LIB_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/lib/*.cpp")
file(GLOB_RECURSE REPO_LIB_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/lib/*.hpp")
file(GLOB_RECURSE REPO_EXEC_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/exec/*.cpp")
file(GLOB_RECURSE REPO_EXEC_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/exec/*.hpp")

add_library(RepoFetcherLib STATIC ${REPO_LIB_SRCS} ${REPO_LIB_HDRS})
target_include_directories(RepoFetcherLib PUBLIC 
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lib>
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/nlohmann_json/single_include>
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmtlib/include>
)
target_compile_definitions(RepoFetcherLib PUBLIC FMT_HEADER_ONLY)
add_executable(RepoFetcher ${REPO_EXEC_SRCS} ${REPO_EXEC_HDRS})
target_include_directories(RepoFetcher PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/exec>)
target_link_libraries(RepoFetcher PRIVATE RepoFetcherLib)

set_property(TARGET RepoFetcherLib PROPERTY CXX_STANDARD 20)
set_property(TARGET RepoFetcher PROPERTY CXX_STANDARD 20)

# TODO: Adicione testes e instale destinos, se necessário.
