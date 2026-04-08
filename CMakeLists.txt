cmake_minimum_required(VERSION 3.10)
project(rest2sql VERSION 1.0)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# ── MariaDB compiler flags — résolu dynamiquement via mariadb_config ─────────
# mariadb_config --cflags retourne tous les -I nécessaires, ex :
#   -I/usr/include/mysql  (RockyLinux)
#   -I/usr/include/mariadb -I/usr/include/mariadb/mysql  (Ubuntu/Debian)
execute_process(
    COMMAND mariadb_config --cflags
    OUTPUT_VARIABLE MARIADB_CFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE MARIADB_CONFIG_RESULT
)

if(NOT MARIADB_CONFIG_RESULT EQUAL 0)
    message(FATAL_ERROR "mariadb_config not found — install MariaDB-devel or mariadb-connector-c-devel")
endif()

message(STATUS "MariaDB cflags: ${MARIADB_CFLAGS}")

# Convertir la string de flags en liste CMake
separate_arguments(MARIADB_CFLAGS_LIST UNIX_COMMAND "${MARIADB_CFLAGS}")

# ── Sources ──────────────────────────────────────────────────────────────────
file(GLOB SOURCES "code/*.c")

# ── Plugin library ───────────────────────────────────────────────────────────
add_library(rest2sql MODULE ${SOURCES})

# Passer tous les flags MariaDB comme options de compilation
target_compile_options(rest2sql PRIVATE ${MARIADB_CFLAGS_LIST})

target_include_directories(rest2sql
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include  # resolves: #include "common.h"
        ${CMAKE_SOURCE_DIR}          # resolves: #include "include/common.h"
)

target_link_libraries(rest2sql cjson microhttpd mysqlservices)

# ── Output ───────────────────────────────────────────────────────────────────
set_target_properties(rest2sql PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "/app/plugin/")

install(TARGETS rest2sql
    LIBRARY DESTINATION "/usr/lib64/mysql/plugin/")
