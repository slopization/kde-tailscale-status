cmake_minimum_required(VERSION 3.16)
project(kde-tailscale-status LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Qml QmlIntegration Test)
include(KDEInstallDirs)
include(ECMQmlModule)

add_library(status_core STATIC src/tailscale_status.cpp)
target_link_libraries(status_core PUBLIC Qt6::Core)

enable_testing()
add_executable(tailscale_status_test tests/tailscale_status_test.cpp)
target_link_libraries(tailscale_status_test PRIVATE status_core Qt6::Test)
add_test(NAME tailscale_status_test COMMAND tailscale_status_test)

add_library(tailscale_status SHARED src/status_backend.cpp)
target_include_directories(tailscale_status PRIVATE src)
target_link_libraries(tailscale_status PUBLIC Qt6::Qml Qt6::QmlIntegration status_core)
ecm_add_qml_module(tailscale_status URI org.kde.tailscale_status GENERATE_PLUGIN_SOURCE)
qt6_query_qml_module(tailscale_status PLUGIN_TARGET plugin_target)
set_target_properties(${plugin_target} PROPERTIES INSTALL_RPATH "$ORIGIN")
install(TARGETS tailscale_status LIBRARY DESTINATION "${KDE_INSTALL_QMLDIR}/org/kde/tailscale_status")
ecm_finalize_qml_module(tailscale_status)
