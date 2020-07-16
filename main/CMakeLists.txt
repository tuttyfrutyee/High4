idf_component_register(SRCS "main.c"
INCLUDE_DIRS "../lib/include" "."
)



target_link_libraries(${COMPONENT_TARGET} "-L ${CMAKE_CURRENT_SOURCE_DIR}/../lib")
target_link_libraries(${COMPONENT_TARGET}
    od
    fr
    fd
    dl
    )