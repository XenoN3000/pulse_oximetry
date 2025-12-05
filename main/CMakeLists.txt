
if(NOT WIN32 OR MSVC)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[m")
    set(ColourBold  "${Esc}[1m")
    set(Red         "${Esc}[31m")
    set(Green       "${Esc}[32m")
    set(Yellow      "${Esc}[33m")
    set(Blue        "${Esc}[34m")
    set(Magenta     "${Esc}[35m")
    set(Cyan        "${Esc}[36m")
    set(White       "${Esc}[37m")
    set(BoldRed     "${Esc}[1;31m")
    set(BoldGreen   "${Esc}[1;32m")
endif()


file(GLOB_RECURSE SOURCES
        "*.c"
        "*.cpp"
)

file(GLOB_RECURSE HEADER_PATHS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.hpp" "*.h" "*/*.hpp" "*/*.h" )

set(INCLUDE_DIR ".")

foreach (header ${HEADER_PATHS})
    get_filename_component(dir "${header}" DIRECTORY)
    message("${Cyan}Processing header: ${header} in directory: ${dir}${ColourReset}")
    list(APPEND INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach ()


set(KCONFIG Kconfig.projbuild)

list(REMOVE_DUPLICATES INCLUDE_DIR)

idf_component_register(
        SRCS ${SOURCES}
        INCLUDE_DIRS ${INCLUDE_DIR}
        REQUIRES nvs_flash esp_driver_gpio esp_event  esp_wifi mqtt esp_driver_i2c wifi_provisioning app_update
)


if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeListsConfigurations.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeListsConfigurations.txt)
    message(STATUS "${Green}Using user-defined CMakeListsConfigurations.txt for configuration.${ColourReset}")
else ()
    message(NOTICE "${Yellow}No CMakeListsConfigurations.txt found, using default configuration.${ColourReset}")
endif ()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeListsUser.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeListsUser.txt)
    message(STATUS "${Green}Using user-defined CMakeListsUser.txt for configuration.${ColourReset}")
else ()
    message(NOTICE "${Yellow}No CMakeListsUser.txt found, using default configuration.${ColourReset}")
endif ()