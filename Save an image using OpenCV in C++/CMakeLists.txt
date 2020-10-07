project(tcamopencvsaveimage)
 
cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)

set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:$ENV{HOME}/.local/lib/pkgconfig")

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
pkg_check_modules(TCAMLIB tcam)

message( ${TCAMLIB_LIBRARIES})

include_directories( ${CMAKE_CURRENT_BINARY_DIR}  ${GSTREAMER_INCLUDE_DIRS} ${TCAMLIB_INCLUDE_DIRS}  ${OpenCV_INCLUDE_DIRS} )
link_directories(${TCAMLIB_LIBRARY_DIRS})
add_definitions(${GSTREAMER_CFLAGS_OTHER})  

add_executable(tcamopencvsaveimage main.cpp tcamcamera.cpp )
add_executable(opencv_gstreamer    opencv_gstreamer.cpp )

target_link_libraries(tcamopencvsaveimage ${TCAMLIB_LIBRARIES} ${GSTREAMER_LIBRARIES} ${OpenCV_LIBS} )
target_link_libraries(opencv_gstreamer                         ${GSTREAMER_LIBRARIES} ${OpenCV_LIBS} )


install(TARGETS tcamopencvsaveimage RUNTIME DESTINATION bin)
 