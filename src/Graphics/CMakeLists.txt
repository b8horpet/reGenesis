#file(GLOB SOURCES ./*.cpp)
#add_subdirectory(Surfaces)
file(GLOB SOURCES ./Surfaces/*.cpp ./SurfaceCommon/*.cpp ./Renderers/*.cpp)
file(GLOB INCLUDES ./)
add_library (Graphics ${SOURCES})
#add_definitions(-lSDL2 -lGL -lGLU)
find_package(OpenGL)
#find_package(SDL2)
find_library(SDL2_LIBRARY NAME SDL2 HINTS SDL2)
target_link_libraries(Graphics ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${SDL2_LIBRARY})
