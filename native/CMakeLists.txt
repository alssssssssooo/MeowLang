cmake_minimum_required(VERSION 3.16)
project(meow C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)
find_package(EXPAT REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

add_executable(meow meow.c)
target_link_libraries(meow PRIVATE
    ZLIB::ZLIB
    EXPAT::EXPAT
    PNG::PNG
    JPEG::JPEG
)

if(UNIX)
    target_link_libraries(meow PRIVATE m)
endif()

if(MSVC)
    target_compile_options(meow PRIVATE /W4)
else()
    target_compile_options(meow PRIVATE -Wall -Wextra)
endif()

install(TARGETS meow RUNTIME DESTINATION bin)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../fonts"
        DESTINATION share/meow)

add_custom_target(install-fonts
    COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/../fonts/install-fonts.sh" --user
    COMMENT "Installing Meow-* symbol fonts for the current user"
    VERBATIM)
