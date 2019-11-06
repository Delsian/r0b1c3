CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
INCLUDE("../config/nrf52.cmake")
PROJECT(controller C ASM)
enable_language(ASM)
enable_language(C)

ADD_DEFINITIONS( 
	-DBOARD_CUSTOM
	-DCONFIG_GPIO_AS_PINRESET
	-DAPP_TIMER_V2
	-DAPP_TIMER_V2_RTC1_ENABLED
	-DFLOAT_ABI_HARD
	-DNRF52840_XXAA
	-DNRF_SD_BLE_API_VERSION=7
	-DS140
	-DSOFTDEVICE_PRESENT
	-DNRF_DFU_SVCI_ENABLED
    -DNRF_DFU_TRANSPORT_BLE=1
	-D__HEAP_SIZE=8192
	-D__STACK_SIZE=8192
	)

INCLUDE("../config/SDK.cmake")

SET(SOURCE_FILES
	src/main.c
	src/bluetooth.c
	src/custom_service.c
	src/control.c
	src/r3_service.c
    src/touchscreen.c
    src/display.c
    ${SDK_DIR}/components/drivers_ext/ili9341/ili9341.c
    ${SDK_DIR}/external/thedotfactory_fonts/orkney8pts.c
    ${SDK_DIR}/external/thedotfactory_fonts/orkney24pts.c
	)

SET(COMPILE_DEFINITIONS_DEBUG -O0 -g3 -DDEBUG)
SET(COMPILE_DEFINITIONS_RELEASE -Os)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/.
	${SDK_INCLUDE_DIRS}
	${SDK_LIB_INCLUDE_DIRS}
	${SDK_BLE_INCLUDE_DIRS}
	${SDK_BOOT_INCLUDE_DIRS}
	${SDK_CRYPTO_INCLUDE_DIRS}
	./src
	../config
	)

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}.elf
	${NRF52_STARTUP}
	${SOURCE_FILES}
	${SDK_LIB_SOURCES}
	${SDK_BLE_SOURCES}
    ${SDK_MOD_DRV_SOURCES}
    ${SDK_LEGACY_SOURCES}
	${NRF52_SYSTEM}
	${SDK_EXT_FPRINTF_SOURCES}
	${SDK_LOG_SOURCES}
	)

TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME}.elf 
	m
	c
	nosys
	)

# Convert elf to hex & bin
execute_process(COMMAND git describe --tags --abbrev=0 OUTPUT_VARIABLE VERSION_TXT)
STRING(REGEX REPLACE "[.\r\n]" "" VERSION ${VERSION_TXT})
MESSAGE("SW version ${VERSION_TXT} as ${VERSION}")
SET(S_PARAM --application-version ${VERSION} --bootloader-version 0 --bl-settings-version 1)
SET(S_APPCMD generate --family ${TARGET_PLATFORM} --application ${CMAKE_PROJECT_NAME}.out)
ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME}.elf 
	POST_BUILD COMMAND ${CMAKE_OBJCOPY} ARGS -R .rtt -Oihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.out
	POST_BUILD COMMAND ${NRFUTIL} settings ${S_APPCMD} ${S_PARAM} ${SETTINGS_HEX}
	POST_BUILD COMMAND ${MERGEHEX} -m ${CMAKE_PROJECT_NAME}.out ${SETTINGS_HEX} -o ${CMAKE_PROJECT_NAME}.hex
	)
ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME}.elf
	POST_BUILD COMMAND ${CMAKE_OBJCOPY} ARGS -R .rtt -Obinary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin
	)

# adds target for erasing and flashing the board
ADD_CUSTOM_TARGET(flashsd
    COMMAND ${NRFJPROG} -f ${TARGET_PLATFORM} --eraseall
    COMMAND ${NRFJPROG} -f ${TARGET_PLATFORM} --program ${SOFTDEVICE_PATH}
    COMMENT "Flashing Softdevice"
    )

ADD_CUSTOM_TARGET(settings
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}	 
    COMMAND ${NRFJPROG} -f ${TARGET_PLATFORM} --program ${SETTINGS_HEX} --sectorerase
    COMMENT "Generate bootloader settings"	
    )

# Create DFU package with version from tag    
SET(KEY_FILE ../config/private.pem)
ADD_CUSTOM_TARGET(dfupkg
    DEPENDS ${CMAKE_PROJECT_NAME}.elf
    COMMAND ${NRFUTIL} pkg generate --hw-version 52 --application ${CMAKE_PROJECT_NAME}.out --key-file ${KEY_FILE} --sd-req 0x9D --application-version-string ${VERSION} dfu.zip  	 
    COMMENT "Generate DFU package ver.${VERSION}"
    )

# Derict FW flashing
ADD_CUSTOM_TARGET(flash
    DEPENDS ${CMAKE_PROJECT_NAME}.elf
    COMMAND ${NRFJPROG} -f ${TARGET_PLATFORM} --program ${CMAKE_PROJECT_NAME}.hex --sectorerase
    COMMAND ${NRFJPROG} -f ${TARGET_PLATFORM} -r
    COMMENT "Flashing program"
    )

ADD_CUSTOM_TARGET(configure
    COMMAND java -jar ${SDK_CONFIG_TOOL} ${SDK_CONFIG_FILE}
    COMMENT "Configure SDK"
    )
