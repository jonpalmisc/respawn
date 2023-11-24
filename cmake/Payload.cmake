set(PAYLOAD_ASSEMBLER ${CMAKE_C_COMPILER})
set(PAYLOAD_ASSEMBLER_FLAGS
  -target arm64-apple-darwin
  -Oz
  -I${CMAKE_SOURCE_DIR}/include
  -DPAYLOAD_SOURCE)

function(add_payload PARENT SOURCE)
  get_filename_component(NAME ${SOURCE} NAME_WE)
  set(HEADER src/payload/${NAME}.h)
  set(OBJECT ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.o)
  set(BINARY ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.bin)

  # The generated header needs to be made a dependency of some other (normal)
  # target so that it is actually generated.
  target_sources(${PARENT} PRIVATE ${HEADER})

  add_custom_command(
    COMMENT "Generating payload header ${HEADER}"
    OUTPUT ${CMAKE_SOURCE_DIR}/${HEADER} DEPENDS ${SOURCE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${PAYLOAD_ASSEMBLER} ${PAYLOAD_ASSEMBLER_FLAGS} -o ${OBJECT} -c ${SOURCE}
    COMMAND segedit ${OBJECT} -extract __TEXT __text ${BINARY}
    COMMAND printf "%s\\n\\n" "// This file is auto-generated. Do not edit!" > ${HEADER}
    COMMAND printf "%s\\n\\n" "#pragma once" >> ${HEADER}
    COMMAND xxd -n ${NAME} -i ${BINARY} >> ${HEADER}
    COMMAND rm ${OBJECT}
    VERBATIM)
endfunction()
