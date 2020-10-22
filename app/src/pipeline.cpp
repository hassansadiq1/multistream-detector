#include "pipeline.h"


Pipeline::Pipeline()
{
    this->context = new ProjectContext();
}

void Pipeline::createElements()
{
/* Create gstreamer elements */
/* Create Pipeline element that will form a connection of other elements */
    pipeline = gst_pipeline_new ("multistream-detector-pipeline");

/* Create nvstreammux instance to form batches from one or more sources. */
    streammux = gst_element_factory_make ("nvstreammux", "stream-muxer");

/* Use nvinfer to infer on batched frame. */
    pgie = gst_element_factory_make ("nvinfer", "primary-nvinference-engine");

/* Add queue elements between every two elements */
    queue1 = gst_element_factory_make ("queue", "queue1");
    queue2 = gst_element_factory_make ("queue", "queue2");
    queue3 = gst_element_factory_make ("queue", "queue3");
    queue4 = gst_element_factory_make ("queue", "queue4");
    queue5 = gst_element_factory_make ("queue", "queue5");

/* Use nvtiler to composite the batched frames into a 2D tiled array based
* on the source of the frames. */
    tiler = gst_element_factory_make ("nvmultistreamtiler", "nvtiler");

/* Use convertor to convert from NV12 to RGBA as required by nvosd */
    nvvidconv = gst_element_factory_make ("nvvideoconvert", "nvvideo-converter");

/* Create OSD to draw on the converted RGBA buffer */
    nvosd = gst_element_factory_make ("nvdsosd", "nv-onscreendisplay");

/* Finally render the osd output */
#ifdef __aarch64__
    transform = gst_element_factory_make ("nvegltransform", "nvegl-transform");
#endif
    sink = gst_element_factory_make ("nveglglessink", "nvvideo-renderer");
}

void Pipeline::Verify()
{
    if (!pipeline || !streammux) {
        g_printerr ("Initial elements could not be created. Exiting.\n");
        exit(-1);
    }

    if (!pgie || !tiler || !nvvidconv || !nvosd || !sink) {
        g_printerr ("Pipeline elements could not be created. Exiting.\n");
        exit(-1);
    }

    if (!queue1 || !queue2 || !queue3 || !queue4 || !queue5) {
        g_printerr ("Queue elements could not be created. Exiting.\n");
        exit(-1);
    }

#ifdef __aarch64__
    if(!transform) {
        g_printerr ("One tegra element could not be created. Exiting.\n");
        exit(-1);
    }
#endif
}

void Pipeline::Configure()
{
    g_object_set (G_OBJECT (streammux), "batch-size", num_sources, NULL);

    g_object_set (G_OBJECT (streammux), "width", context->MUXER_OUTPUT_WIDTH, "height",
        context->MUXER_OUTPUT_HEIGHT,
        "batched-push-timeout", context->MUXER_BATCH_TIMEOUT_USEC, NULL);

    /* Configure the nvinfer element using the nvinfer config file. */
    g_object_set (G_OBJECT (pgie),
        "config-file-path", PGIE_CONFIG_FILE, NULL);

    /* Override the batch-size set in the config file with the number of sources. */
    g_object_get (G_OBJECT (pgie), "batch-size", &pgie_batch_size, NULL);
    if (pgie_batch_size < num_sources) {
        g_printerr
            ("WARNING: Overriding infer-config batch-size (%d) with number of sources (%d)\n",
            pgie_batch_size, num_sources);
        g_object_set (G_OBJECT (pgie), "batch-size", num_sources, NULL);
    }

    tiler_rows = (guint) sqrt (num_sources);
    tiler_columns = (guint) ceil (1.0 * num_sources / tiler_rows);
    /* we set the tiler properties here */
    g_object_set (G_OBJECT (tiler), "rows", tiler_rows, "columns", tiler_columns,
        "width", context->TILED_OUTPUT_WIDTH, "height", context->TILED_OUTPUT_HEIGHT, NULL);

    g_object_set (G_OBJECT (nvosd), "process-mode", OSD_PROCESS_MODE,
        "display-text", OSD_DISPLAY_TEXT, NULL);

    g_object_set (G_OBJECT (sink), "qos", 0, "sync", 0, NULL);
}

void Pipeline::ConstructPipeline()
{
  gst_bin_add (GST_BIN (pipeline), streammux);

/* Set up the pipeline */
/* we add all elements into the pipeline */
#ifdef __aarch64__
    gst_bin_add_many (GST_BIN (pipeline), queue1, pgie, queue2, tiler, queue3,
        nvvidconv, queue4, nvosd, queue5, transform, sink, NULL);
    /* we link the elements together
    * nvstreammux -> nvinfer -> nvtiler -> nvvidconv -> nvosd -> video-renderer */
    if (!gst_element_link_many (streammux, queue1, pgie, tiler,
            nvvidconv, nvosd, transform, sink, NULL)) {
        g_printerr ("Elements could not be linked. Exiting.\n");
        exit(-1);
    }
#else
    gst_bin_add_many (GST_BIN (pipeline), queue1, pgie, queue2, tiler, queue3,
        nvvidconv, queue4, nvosd, queue5, sink, NULL);
    /* we link the elements together
    * nvstreammux -> nvinfer -> nvtiler -> nvvidconv -> nvosd -> video-renderer */
    if (!gst_element_link_many (streammux, queue1, pgie, queue2, tiler, queue3,
            nvvidconv, queue4, nvosd, queue5, sink, NULL)) {
        g_printerr ("Elements could not be linked. Exiting.\n");
        exit(-1);
    }
#endif
}

void Pipeline::RunPipelineAsync()
{
    // and source count is greater than zero
    while(1){
        if (num_sources > 0)
        {
            // Set the pipeline to "playing" state
            g_print("Setting pipeline state to Playing");
            gst_element_set_state(this->pipeline, GST_STATE_PLAYING);
            g_main_loop_run(this->loop);
        }
    }
    return;
}

Pipeline::~Pipeline()
{
}
