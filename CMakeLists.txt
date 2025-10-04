cmake_minimum_required(VERSION 3.16)
project(StudentOfficeDBMS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_MOCK_DB "Build without MySQL/MariaDB client linkage" OFF)

add_executable(student_office main.cpp)

if (USE_MOCK_DB)
  target_compile_definitions(student_office PRIVATE USE_MOCK_DB)
else()
  find_package(PkgConfig)
  set(MARIADB_FOUND FALSE)
  if (PkgConfig_FOUND)
    pkg_check_modules(MARIADB libmariadb)
    if (MARIADB_FOUND)
      target_include_directories(student_office PRIVATE ${MARIADB_INCLUDE_DIRS})
      target_link_libraries(student_office PRIVATE ${MARIADB_LINK_LIBRARIES})
    endif()
  endif()

  if (NOT MARIADB_FOUND)
    # Fallback to manual include/lib search for MariaDB/MySQL
    find_path(MYSQL_INCLUDE_DIR mysql.h PATH_SUFFIXES mysql mariadb PATHS
      /usr/include /usr/local/include /opt/homebrew/include /opt/local/include)
    find_library(MYSQL_LIBRARY NAMES mariadb mysqlclient PATHS
      /usr/lib /usr/local/lib /opt/homebrew/lib /opt/local/lib)

    if (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
      target_include_directories(student_office PRIVATE ${MYSQL_INCLUDE_DIR})
      target_link_libraries(student_office PRIVATE ${MYSQL_LIBRARY})
    else()
      message(FATAL_ERROR "Could not find MariaDB/MySQL client dev files. Enable -DUSE_MOCK_DB=ON or install libmariadb-dev.")
    endif()
  endif()
endif()

install(TARGETS student_office RUNTIME DESTINATION bin)
