#ifndef PROJECT_PIPELINE_H
#define PROJECT_PIPELINE_H

#include <glib.h>
#include <gst/gst.h>
#include <math.h>
#include <thread>
#include "gstnvdsmeta.h"

#include "context.h"

using namespace std;

class Pipeline
{
public:

    GMainLoop *loop = NULL;
    GstElement *pipeline = NULL, *streammux = NULL, *sink = NULL, *pgie = NULL,
        *queue1, *queue2, *queue3, *queue4, *queue5, *nvvidconv = NULL,
        *nvosd = NULL, *tiler = NULL;
#ifdef __aarch64__
    GstElement *transform = NULL;
#endif
    GstBus *bus = NULL;
    guint bus_watch_id;
    GstPad *tiler_src_pad = NULL;
    guint i, num_sources;
    guint tiler_rows, tiler_columns;
    guint pgie_batch_size;

    sourceManager source_manager;
    ProjectContext *context;

    void createElements();

    void Verify();

    void Configure();

    void ConstructPipeline();

    void stopPipeline();

    void RunPipelineAsync();

    static gboolean BusCall(GstBus *bus, GstMessage *msg, gpointer data);

    Pipeline();
    
    ~Pipeline();
};

#endif //PROJECT_PIPELINE_H
