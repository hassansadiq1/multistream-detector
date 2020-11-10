#ifndef PARENTPROJECT_CONTEXT_H
#define PARENTPROJECT_CONTEXT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <pthread.h>

#define PGIE_CONFIG_FILE "../../configs/pgie_detector.txt"
#define SOURCE_PROPERTIES  "../../configs/source_properties.ini"
#define TRACKER_CONFIG_FILE (char*)"../../configs/tracker_config.txt"

#define MAX_DISPLAY_LEN 64
#define PGIE_CLASS_ID_VEHICLE 0
#define PGIE_CLASS_ID_BICYCLE 1
#define PGIE_CLASS_ID_PERSON 2
#define PGIE_CLASS_ID_ROADSIGN 3

#define CONFIG_GROUP_TRACKER "tracker"
#define CONFIG_GROUP_TRACKER_WIDTH "tracker-width"
#define CONFIG_GROUP_TRACKER_HEIGHT "tracker-height"
#define CONFIG_GROUP_TRACKER_LL_CONFIG_FILE "ll-config-file"
#define CONFIG_GROUP_TRACKER_LL_LIB_FILE "ll-lib-file"
#define CONFIG_GROUP_TRACKER_ENABLE_BATCH_PROCESS "enable-batch-process"

/* Tracker config parsing */

#define CHECK_ERROR(error) \
    if (error) { \
        g_printerr ("Error while parsing config file: %s\n", error->message); \
        goto done; \
    }

/* By default, OSD process-mode is set to CPU_MODE. To change mode, set as:
 * 1: GPU mode (for Tesla only)
 * 2: HW mode (For Jetson only)
 */
#define OSD_PROCESS_MODE 0

/* By default, OSD will not display text. To display text, change this to 1 */
#define OSD_DISPLAY_TEXT 0

/* NVIDIA Decoder source pad memory feature. This feature signifies that source
 * pads having this capability will push GstBuffers containing cuda buffers. */
#define GST_CAPS_FEATURES_NVMM "memory:NVMM"

using namespace std;

class SourceProperties{
    public:
    string uri;
    int source_id;
    int TopLeftX;
    int TopLeftY;
    int BottomRightX;
    int BottomRightY;
};

class sourceManager{
    public:
    std::vector<SourceProperties*> allSources;
    std::vector<int> allSourcesStatus;
    int num_sources = 0;
    pthread_mutex_t sourceMapMutex;

    bool sourceExists(int source_id);
    SourceProperties* getSourceProperties(int source_id);

};

class ProjectContext
{
public:
    ProjectContext();

/* The muxer output resolution must be set if the input streams will be of
 * different resolution. The muxer will scale all the input frames to this
 * resolution. */
    int MUXER_OUTPUT_WIDTH, MUXER_OUTPUT_HEIGHT, MUXER_BATCH_SIZE;

/* Muxer batch formation timeout, for e.g. 40 millisec. Should ideally be set
 * based on the fastest source's framerate. */
    int MUXER_BATCH_TIMEOUT_USEC;

    int TILED_OUTPUT_WIDTH, TILED_OUTPUT_HEIGHT;

    void loadConfig(char *str1, char *str2);
    void loadStrConfig(char *str1, string& str2);
    void loadSourceProperties(SourceProperties *properties, int i);

    ~ProjectContext();
};

#endif
