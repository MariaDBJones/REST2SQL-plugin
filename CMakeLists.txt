cmake_minimum_required(VERSION 3.10)
project(rest2sql VERSION 1.0)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Include directories
# old school include (afaik)
#include_directories(/usr/include/mysql/server/)


# Optionally, if you want to include all .c files in the code directory:
file(GLOB SOURCES "code/*.c")

# Add the plugin library
add_library(rest2sql MODULE ${SOURCES})

# new school include (afaik)
target_include_directories(rest2sql
    PRIVATE
        /usr/include/mysql/server
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(rest2sql cjson microhttpd mysqlservices)

# Set output directory for the plugin
set_target_properties(rest2sql PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "/app/plugin/" )

# Installation rules
install(TARGETS rest2sql
    LIBRARY DESTINATION "/usr/lib64/mysql/plugin/" 
    )
