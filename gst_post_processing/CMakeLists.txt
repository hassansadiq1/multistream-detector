cmake_minimum_required(VERSION 3.4)
project(gst_post_processing)

include(FindPkgConfig)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g3 -Wl,-no-undefined")
set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)

include_directories(${PROJECT_SOURCE_DIR}/include)

find_package(PkgConfig) #finding pkg-config is a helper tool

#using pkg-config to getting Gstreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-video-1.0 gstreamer-audio-1.0
        gstreamer-base-1.0 gstreamer-plugins-base-1.0)

#including GStreamer header files directory
include_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
        ${GStreamer_base_INCLUDE_DIRS}       
)

#linking GStreamer library directory
link_directories(
        ${GLIB_LIBRARY_DIRS}
        ${GSTREAMER_LIBRARY_DIRS}
        ${GStreamer_base_LIBRARIES}
        ${GStreamer_plugins_base_LIBRARIES}
        ${GStreamer_video_LIBRARIES}
)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package(CUDA 10.2 REQUIRED)
include_directories(/usr/local/cuda/include)
link_directories(/usr/local/cuda/lib64/)

include_directories(/opt/nvidia/deepstream/deepstream-5.0/sources/includes/)
link_directories(/opt/nvidia/deepstream/deepstream-5.0)
link_directories(/opt/nvidia/deepstream/deepstream-5.0/lib)
link_directories(/usr/lib/aarch64-linux-gnu)
set(NVIDIA_DEEPSTREAM_LIBS cudart nvdsgst_meta nvds_meta nvdsgst_helper nvds_utils
 nvbufsurface nvbufsurftransform nvinfer nvparsers nvinfer_plugin nvonnxparser
 nvds_meta nvbufsurface nvbufsurftransform)

AUX_SOURCE_DIRECTORY(src SRC)
add_library(gst_post_processing SHARED ${SRC})

target_link_libraries(gst_post_processing ${OpenCV_LIBS} )
target_link_libraries(gst_post_processing ${NVIDIA_DEEPSTREAM_LIBS})
target_link_libraries(gst_post_processing 
        ${GLIB_LIBRARY_DIRS}
        ${GSTREAMER_LIBRARY_DIRS}
        ${GStreamer_base_LIBRARIES}
        ${GStreamer_plugins_base_LIBRARIES}
        ${GStreamer_video_LIBRARIES}
)

install(TARGETS gst_post_processing DESTINATION /opt/nvidia/deepstream/deepstream-5.0/lib/gst-plugins/)