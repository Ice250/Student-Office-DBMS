cmake_minimum_required(VERSION 3.16)
project(StudentOffice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automatic processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Prefer Qt6, fallback to Qt5
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

# Locate MySQL client (best-effort)
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
  PATHS /usr/include /usr/include/mysql /usr/local/include /usr/local/include/mysql
)
find_library(MYSQL_LIBRARY NAMES mysqlclient
  PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
)
if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
  message(STATUS "Found MySQL: ${MYSQL_LIBRARY}")
else()
  message(WARNING "MySQL headers or library not found. Build may fail if DB code is compiled.")
endif()

add_executable(student_office
  main.cpp
  LoginDialog.cpp
)

target_include_directories(student_office PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${MYSQL_INCLUDE_DIR}
)

if(Qt6_FOUND)
  target_link_libraries(student_office PRIVATE Qt6::Widgets)
else()
  target_link_libraries(student_office PRIVATE Qt5::Widgets)
endif()

if(MYSQL_LIBRARY)
  target_link_libraries(student_office PRIVATE ${MYSQL_LIBRARY})
endif()
