project(truffle_object CXX)
add_library(${PROJECT_NAME}
    image.cpp
    text.cpp
    button.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    absl::strings
    absl::flat_hash_map
    absl::str_format
    truffle_common
    truffle_engine
)
