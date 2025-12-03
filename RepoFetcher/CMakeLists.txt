# CMakeList.txt: projeto de CMake para RepoFetcher, incluir origem e definir
# lógica específica a um projeto aqui.
#

# Adicione a origem ao executável deste projeto.

file(GLOB_RECURSE REPO_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
file(GLOB_RECURSE REPO_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")

add_executable(RepoFetcher ${REPO_SRCS} ${REPO_HDRS})
target_include_directories(RepoFetcher PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/vendor/nlohmann_json/single_include)
target_include_directories(RepoFetcher PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/vendor/fmtlib/include)
target_compile_definitions(RepoFetcher PRIVATE FMT_HEADER_ONLY)

set_property(TARGET RepoFetcher PROPERTY CXX_STANDARD 20)

# TODO: Adicione testes e instale destinos, se necessário.
