cmake_minimum_required(VERSION 2.6)
project(OpenOMF C)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)
find_package(SDL2)
find_package(shadowdive)
find_package(dumb)
find_package(opengl)
find_package(openal)

set(CMAKE_C_FLAGS "-Wall -std=c99 -ggdb -Werror -DDEBUGMODE")
set(CMAKE_C_FLAGS_DEBUG "-Wall -std=c99 -ggdb -Werror -DDEBUGMODE")
set(CMAKE_C_FLAGS_RELEASE "-Wall -std=c99 -O2 -march=i686 -Werror")

add_executable(openomf 
    src/main.c
    src/engine.c
    src/utils/log.c
    src/video/video.c
    src/video/texture.c
    src/video/fbo.c
    src/video/shader.c
    src/audio/audio.c
    src/audio/music.c
    src/audio/sound.c
)

set(COREINCS
    include
    ${SDL2_INCLUDE_DIR} 
    ${SHADOWDIVE_INCLUDE_DIR} 
    ${DUMB_INCLUDE_DIR} 
    ${OPENAL_INCLUDE_DIR} 
    ${OPENGL_INCLUDE_DIR}
)

set(CORELIBS 
    ${SDL2_LIBRARY} 
    ${SHADOWDIVE_LIBRARY} 
    ${DUMB_LIBRARY} 
    ${OPENGL_LIBRARY} 
    ${OPENAL_LIBRARY}
)

IF(MINGW)
    set(CORELIBS mingw32 ${CORELIBS})
ENDIF(MINGW)

include_directories(${COREINCS})
target_link_libraries(openomf ${CORELIBS})