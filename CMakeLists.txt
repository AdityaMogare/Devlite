cmake_minimum_required(VERSION 3.24)
project(devlite VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0 CACHE STRING "Minimum macOS version")
else()
  message(WARNING
    "devlite v1 targets macOS only. Other platforms may build but are untested "
    "and unsupported; see SPEC section 1.")
endif()

include(CTest)
include(GNUInstallDirs)

# Homebrew builds with DEVLITE_FETCH_DEPS=OFF and the bottled CLI11 and
# nlohmann-json packages. Local builds keep the FetchContent default.
option(DEVLITE_FETCH_DEPS
  "Download CLI11, nlohmann/json, and GoogleTest with FetchContent" ON)

if(DEVLITE_FETCH_DEPS)
  include(FetchContent)
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git GIT_TAG v2.4.2 GIT_SHALLOW TRUE)
  FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3 GIT_SHALLOW TRUE)
  set(devlite_fetched cli11 json)
  if(BUILD_TESTING)
    FetchContent_Declare(googletest
      GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.15.2 GIT_SHALLOW TRUE)
    list(APPEND devlite_fetched googletest)
  endif()
  FetchContent_MakeAvailable(${devlite_fetched})
else()
  find_package(CLI11 CONFIG REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)
  if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
  endif()
endif()

add_library(devlite_core
  src/platform/environment.cpp
  src/platform/path_resolver.cpp
  src/platform/process_runner_posix.cpp
  src/probes/platform_probe.cpp
  src/probes/path_probe.cpp
  src/probes/git_probe.cpp
  src/probes/python_probe.cpp
  src/probes/repo_probe.cpp
  src/probes/probe_runner.cpp
  src/rules/path_rules.cpp
  src/rules/git_rules.cpp
  src/rules/python_rules.cpp
  src/rules/venv_rules.cpp
  src/rules/repo_rules.cpp
  src/rules/registry.cpp
  src/format/text_formatter.cpp
  src/format/json_formatter.cpp)
target_include_directories(devlite_core PUBLIC include)
target_link_libraries(devlite_core PUBLIC nlohmann_json::nlohmann_json)
target_compile_definitions(devlite_core PUBLIC DEVLITE_VERSION="${PROJECT_VERSION}")

add_executable(devlite src/main.cpp)
target_link_libraries(devlite PRIVATE devlite_core CLI11::CLI11)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(TARGETS devlite RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
