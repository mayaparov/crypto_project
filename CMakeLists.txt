cmake_minimum_required(VERSION 3.16)
project(CNetworkWrapperProject)

# Явное создание директорий для MOC
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/CNetworkWrapper_autogen)

set(CMAKE_OSX_DEPLOYMENT_TARGET "14.7")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Network REQUIRED)

# Библиотека
add_library(CNetworkWrapper STATIC
    src/AuthHandler.cpp
    src/CoursesHandler.cpp
    src/HandlerFactory.cpp
    src/CNetworkWrapper.cpp
    src/ErrorHandler.cpp
    src/RegistrationHandler.cpp

    include/AuthHandler.h
    include/CoursesHandler.h
    include/HandlerFactory.h
    include/ResponseHandler.h
    include/CNetworkWrapper.h
    include/ErrorHandler.h
    include/RegistrationHandler.h
)

# Настройка путей
target_include_directories(CNetworkWrapper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/CNetworkWrapper_autogen
)

target_link_libraries(CNetworkWrapper
    Qt6::Core
    Qt6::Network
)

# Тестовое приложение
add_executable(TestClient test/main.cpp)
target_link_libraries(TestClient CNetworkWrapper)
