cmake_minimum_required(VERSION 3.4)
project(nvdsgst_dsexample)

include(FindPkgConfig)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g3 -Wl,-no-undefined")
set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)

include_directories(PUBLIC ${PROJECT_SOURCE_DIR}/include/)

include_directories(/opt/nvidia/deepstream/deepstream-5.0/sources/includes/)
link_directories(/opt/nvidia/deepstream/deepstream-5.0/sources/includes/)

find_package(PkgConfig)
link_directories(/usr/lib/aarch64-linux-gnu/gstreamer-1.0)
pkg_search_module(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-video-1.0
        gstreamer-base-1.0 gstreamer-plugins-base-1.0)

include_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
        ${GStreamer_base_INCLUDE_DIRS}
        ${GStreamer_plugins_bad_INCLUDE_DIRS}
        ${GStreamer_video_INCLUDE_DIRS}
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

link_directories(/usr/lib/aarch64-linux-gnu)
link_directories(/usr/local/deepstream)
link_directories(/opt/nvidia/deepstream/deepstream-5.0/lib)
set(NVIDIA_DEEPSTREAM_LIBS cudart nvdsgst_meta nvds_meta nvdsgst_helper nvds_utils
 nvbufsurface nvbufsurftransform nvinfer nvparsers nvinfer_plugin nvonnxparser
 nvds_meta nvbufsurface nvbufsurftransform)

include_directories(/usr/include/glib-2.0/)
include_directories(/usr/lib/aarch64-linux-gnu/glib-2.0/include)
set(G_Related_LIBS gobject-2.0 gmodule-2.0 gthread-2.0 glib-2.0)
 
pkg_search_module(GStreamer REQUIRED gstreamer-1.0)
pkg_search_module(GStreamer_base REQUIRED gstreamer-base-1.0)
pkg_search_module(GStreamer_plugins_base REQUIRED gstreamer-plugins-base-1.0)
pkg_search_module(GStreamer_video REQUIRED gstreamer-video-1.0)
include_directories(${GStreamer_INCLUDE_DIRS})
include_directories(${GStreamer_base_INCLUDE_DIRS})
include_directories(${GStreamer_plugins_bad_INCLUDE_DIRS})
include_directories(${GStreamer_video_INCLUDE_DIRS})

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
 
include_directories(${PROJECT_SOURCE_DIR}/../app/include)
AUX_SOURCE_DIRECTORY(src SRC)
add_library(nvdsgst_dsexample SHARED ${SRC})

target_link_libraries(nvdsgst_dsexample ${NVIDIA_DEEPSTREAM_LIBS})
target_link_libraries(nvdsgst_dsexample 
        ${GLIB_LIBRARY_DIRS}
        ${GSTREAMER_LIBRARY_DIRS}
        ${GStreamer_base_LIBRARIES}
        ${GStreamer_plugins_base_LIBRARIES}
        ${GStreamer_video_LIBRARIES}
        ${OpenCV_LIBS}
        ${G_Related_LIBS}
        -lzmq
)

install(TARGETS nvdsgst_dsexample DESTINATION /opt/nvidia/deepstream/deepstream-5.0/lib/gst-plugins/)
