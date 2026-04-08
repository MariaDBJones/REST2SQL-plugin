cmake_minimum_required(VERSION 3.10)
project(rest2sql VERSION 1.0)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# ── MariaDB include path — résolu dynamiquement via mariadb_config ──────────
# mariadb_config --include retourne ex: -I/usr/include/mysql
# on extrait le chemin en strippant le -I initial
execute_process(
    COMMAND mariadb_config --include
    OUTPUT_VARIABLE MARIADB_INCLUDE_FLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE MARIADB_CONFIG_RESULT
)

if(NOT MARIADB_CONFIG_RESULT EQUAL 0)
    message(FATAL_ERROR "mariadb_config not found — install MariaDB-devel or mariadb-connector-c-devel")
endif()

# Transformer "-I/usr/include/mysql" en "/usr/include/mysql"
string(REGEX REPLACE "^-I" "" MARIADB_INCLUDE_DIR "${MARIADB_INCLUDE_FLAGS}")
message(STATUS "MariaDB include dir: ${MARIADB_INCLUDE_DIR}")

# ── Sources ──────────────────────────────────────────────────────────────────
file(GLOB SOURCES "code/*.c")

# ── Plugin library ───────────────────────────────────────────────────────────
add_library(rest2sql MODULE ${SOURCES})

target_include_directories(rest2sql
    PRIVATE
        ${MARIADB_INCLUDE_DIR}           # headers MariaDB (résolu par mariadb_config)
        ${MARIADB_INCLUDE_DIR}/server    # headers internes serveur
        ${CMAKE_SOURCE_DIR}/include      # resolves: #include "common.h"
        ${CMAKE_SOURCE_DIR}              # resolves: #include "include/common.h"
)

target_link_libraries(rest2sql cjson microhttpd mysqlservices)

# ── Output ───────────────────────────────────────────────────────────────────
set_target_properties(rest2sql PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "/app/plugin/")

install(TARGETS rest2sql
    LIBRARY DESTINATION "/usr/lib64/mysql/plugin/")
