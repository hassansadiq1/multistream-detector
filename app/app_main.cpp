#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <thread>
#include <unistd.h>

#include "gstnvdsmeta.h"
#ifndef PLATFORM_TEGRA
#include "gst-nvmessage.h"
#endif

#include "context.h"
#include "pipeline.h"

bool running;
sourceManager srcmanager;
int sourcebinCount = 0;
void checkInactiveSource(vector<int> source_ids){

    for(int i = 0; i < source_ids.size(); i++){
        srcmanager.allSources[source_ids[i]]->consectiveMissedFrames = 0;
        srcmanager.allSources[source_ids[i]]->firstFrame = 1;
    }
    for (int i = 0; i < srcmanager.allSources.size(); i++){
        if (srcmanager.allSources[i]->firstFrame){
            srcmanager.allSources[i]->consectiveMissedFrames++;
        }
        if (srcmanager.allSources[i]->consectiveMissedFrames > 80){
            if (srcmanager.allSourcesStatus[i] == 1){
                cout << "Got End of Stream from: " << srcmanager.allSources[i]->uri<<endl;
                srcmanager.allSourcesStatus[i] = 2;
                srcmanager.allSources[i]->firstFrame = 0;
            } else
                srcmanager.allSources[i]->consectiveMissedFrames = 0;
        }
    }
}


/* tiler_sink_pad_buffer_probe  will extract metadata received on OSD sink pad
* and update params for drawing rectangle, object information etc. */

static GstPadProbeReturn
tiler_src_pad_buffer_probe (GstPad * pad, GstPadProbeInfo * info,
    gpointer u_data)
{
    GstBuffer *buf = (GstBuffer *) info->data;
    guint num_rects = 0; 
    NvDsObjectMeta *obj_meta = NULL;
    guint vehicle_count = 0;
    guint person_count = 0;
    NvDsMetaList * l_frame = NULL;
    NvDsMetaList * l_obj = NULL;
    //NvDsDisplayMeta *display_meta = NULL;

    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buf);
    vector<int> batch_meta_frame_ids;
    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL;
            l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);
        batch_meta_frame_ids.push_back(frame_meta->source_id);
    }
    checkInactiveSource(batch_meta_frame_ids);
    return GST_PAD_PROBE_OK;
}

vector<string> stringtoVector(string str, char separator){
    vector<string> v;
    stringstream ss(str);
 
    while (ss.good()) {
        string substr;
        getline(ss, substr, separator);
        v.push_back(substr);
    }
    return v;
}

static gboolean
bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *) data;
    switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
        g_print ("End of stream\n");
        g_main_loop_quit (loop);
        break;
    case GST_MESSAGE_WARNING:
    {
        gchar *debug;
        GError *error;
        gst_message_parse_warning (msg, &error, &debug);
        g_printerr ("WARNING from element %s: %s\n",
            GST_OBJECT_NAME (msg->src), error->message);
        g_free (debug);
        g_printerr ("Warning: %s\n", error->message);
        g_error_free (error);
        break;
    }
    case GST_MESSAGE_ERROR:
    {
        gchar *debug;
        GError *error;
        gst_message_parse_error (msg, &error, &debug);
        g_printerr ("ERROR from element %s: %s\n",
            GST_OBJECT_NAME (msg->src), error->message);
        if (debug)
            g_printerr ("Error details: %s\n", debug);

        g_free (debug);
        g_error_free (error);
        g_main_loop_quit (loop);
        break;
    }
    default:
         break;
    }
  return TRUE;
}

static void
cb_newpad (GstElement * decodebin, GstPad * decoder_src_pad, gpointer data)
{
    g_print ("In cb_newpad\n");
    GstCaps *caps = gst_pad_get_current_caps (decoder_src_pad);
    const GstStructure *str = gst_caps_get_structure (caps, 0);
    const gchar *name = gst_structure_get_name (str);
    GstElement *source_bin = (GstElement *) data;
    GstCapsFeatures *features = gst_caps_get_features (caps, 0);

    /* Need to check if the pad created by the decodebin is for video and not
    * audio. */
    if (!strncmp (name, "video", 5)) {
        /* Link the decodebin pad only if decodebin has picked nvidia
        * decoder plugin nvdec_*. We do this by checking if the pad caps contain
        * NVMM memory features. */
        if (gst_caps_features_contains (features, GST_CAPS_FEATURES_NVMM)) {
        /* Get the source bin ghost pad */
        GstPad *bin_ghost_pad = gst_element_get_static_pad (source_bin, "src");
        if (!gst_ghost_pad_set_target (GST_GHOST_PAD (bin_ghost_pad),
                decoder_src_pad)) {
            g_printerr ("Failed to link decoder src pad to source bin ghost pad\n");
        }
        gst_object_unref (bin_ghost_pad);
        } else {
            g_printerr ("Error: Decodebin did not pick nvidia decoder plugin.\n");
        }
    }
}

static void
decodebin_child_added (GstChildProxy * child_proxy, GObject * object,
    gchar * name, gpointer user_data)
{
    g_print ("Decodebin child added: %s\n", name);
    if (g_strrstr (name, "decodebin") == name) {
        g_signal_connect (G_OBJECT (object), "child-added",
            G_CALLBACK (decodebin_child_added), user_data);
    }
}

static GstElement *
create_source_bin (guint index, gchar * uri)
{
    GstElement *bin = NULL, *uri_decode_bin = NULL;
    gchar bin_name[16] = { };

    g_snprintf (bin_name, 15, "source-bin-%02d", index);
    /* Create a source GstBin to abstract this bin's content from the rest of the
    * pipeline */
    bin = gst_bin_new (bin_name);

    /* Source element for reading from the uri.
    * We will use decodebin and let it figure out the container format of the
    * stream and the codec and plug the appropriate demux and decode plugins. */
    uri_decode_bin = gst_element_factory_make ("uridecodebin", "uri-decode-bin");

    if (!bin || !uri_decode_bin) {
        g_printerr ("One element in source bin could not be created.\n");
        return NULL;
    }

    /* We set the input uri to the source element */
    g_object_set (G_OBJECT (uri_decode_bin), "uri", uri, NULL);

    /* Connect to the "pad-added" signal of the decodebin which generates a
    * callback once a new pad for raw data has beed created by the decodebin */
    g_signal_connect (G_OBJECT (uri_decode_bin), "pad-added",
        G_CALLBACK (cb_newpad), bin);
    g_signal_connect (G_OBJECT (uri_decode_bin), "child-added",
        G_CALLBACK (decodebin_child_added), bin);

    gst_bin_add (GST_BIN (bin), uri_decode_bin);

    /* We need to create a ghost pad for the source bin which will act as a proxy
    * for the video decoder src pad. The ghost pad will not have a target right
    * now. Once the decode bin creates the video decoder and generates the
    * cb_newpad callback, we will set the ghost pad target to the video decoder
    * src pad. */
    if (!gst_element_add_pad (bin, gst_ghost_pad_new_no_target ("src",
                GST_PAD_SRC))) {
        g_printerr ("Failed to add ghost pad in source bin\n");
        return NULL;
    }

    return bin;
}

int Execute_Command( const std::string&  command,
                     std::string&        output,
                     const std::string&  mode = "r")
{
    // Create the stringstream
    std::stringstream sout;

    // Run Popen
    FILE *in;
    char buff[512];

    // Test output
    if(!(in = popen(command.c_str(), mode.c_str()))){
        return 1;
    }

    // Parse output
    while(fgets(buff, sizeof(buff), in)!=NULL){
        sout << buff;
    }

    // Close
    int exit_code = pclose(in);

    // set output
    output = sout.str();

    // Return exit code
    return exit_code;
}

bool ping_ip_cam(string uri){
    std::string command = "ffmpeg -y -i " + uri + " -vframes 1 do.jpg 2>&1";

    std::string details;

    // Execute the ping command
    int code = Execute_Command(command, details);

    return (code == 0);
}

int
main (int argc, char *argv[])
{
    char temp_char[150];

    /* Standard GStreamer initialization */
    gst_init (&argc, &argv);
    running = TRUE;

    Pipeline pipeline;
    thread PipelineExecutor;

    pipeline.loop = g_main_loop_new (NULL, FALSE);
    g_mutex_init(&pipeline.Lock1);

    pipeline.context->loadConfig((char *) "num_sources", temp_char);
    pipeline.num_sources = atoi(temp_char);

    pipeline.createElements();
    pipeline.Verify();
    pipeline.Configure();

    pipeline.context->loadConfig((char *) "timeout", temp_char);
    int notifyTimeout = atoi(temp_char);

    pipeline.context->loadConfig((char *) "images_path", temp_char);
    g_object_set (G_OBJECT (pipeline.postprocessing),
        "Images-Path", temp_char,
        "source-manager", &srcmanager,
        "notify-timeout", notifyTimeout,
        NULL);


    /* we add a message handler */
    pipeline.bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline.pipeline));
    pipeline.bus_watch_id = gst_bus_add_watch (pipeline.bus, bus_call, pipeline.loop);
    gst_object_unref (pipeline.bus);

    pipeline.ConstructPipeline();

    /* Lets add probe to get informed of the meta data generated, we add probe to
        * the sink pad of the osd element, since by that time, the buffer would have
        * had got all the metadata. */
    pipeline.tiler_src_pad = gst_element_get_static_pad (pipeline.tiler, "sink");
    if (!pipeline.tiler_src_pad)
        g_print ("Unable to get src pad\n");
    else
        gst_pad_add_probe (pipeline.tiler_src_pad, GST_PAD_PROBE_TYPE_BUFFER,
            tiler_src_pad_buffer_probe, NULL, NULL);
    gst_object_unref (pipeline.tiler_src_pad);

    PipelineExecutor = thread(&Pipeline::RunPipelineAsync, pipeline);
    std::cout<<"No. of sources: " << pipeline.num_sources << endl;

    for (int i = 0; i < pipeline.num_sources; i++) {
        if (srcmanager.num_sources >= pipeline.context->MUXER_BATCH_SIZE)
        {
            std::cout << "Muxer batch size reached\n";
            continue;
        }
        SourceProperties* src = new SourceProperties;
        pipeline.context->loadSourceProperties(src, i);
        std::cout << "Adding: " << src->uri << endl;
        srcmanager.allSources.push_back(src);
        srcmanager.allSourcesStatus.push_back(1);
        srcmanager.num_sources++;

        GstPad *sinkpad, *srcpad;
        gchar pad_name[16] = { };

        GstElement *source_bin = create_source_bin (i, (char *)src->uri.c_str());
        sourcebinCount++;
        if (!source_bin) {
            g_printerr ("Failed to create source bin. Adding to Ping list again.\n");
            continue;
        }

        gst_bin_add (GST_BIN (pipeline.pipeline), source_bin);

        g_snprintf (pad_name, 15, "sink_%u", i);
        sinkpad = gst_element_get_request_pad (pipeline.streammux, pad_name);
        if (!sinkpad) {
            g_printerr ("Streammux request resink pad failed. Adding to Ping list again.\n");
            continue;
        }

        srcpad = gst_element_get_static_pad (source_bin, "src");
        if (!srcpad) {
            g_printerr ("Failed to get src pad of source bin. Exiting. Adding to Ping list again\n");
            continue;
        }

        if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK) {
            g_printerr ("Failed to link source bin to restream muxer. Adding to Ping list again.\n");
            continue;
        }

        gst_object_unref (srcpad);
        gst_object_unref (sinkpad);

        GstStateChangeReturn stateChange;
        stateChange = gst_element_set_state(source_bin, GST_STATE_PLAYING);

        switch (stateChange)
        {
            case GST_STATE_CHANGE_SUCCESS:
                g_printerr("State change: Success");
                break;

            case GST_STATE_CHANGE_FAILURE:
                g_printerr("State change: Failure");
                srcmanager.allSourcesStatus[i] = 2;
                break;
            default:
                break;
        }
        pipeline.sourcebins.push_back(source_bin);
        sleep(1);
    }

    while(running){
        for (int i = 0; i < srcmanager.allSources.size(); i++){
            if(srcmanager.allSourcesStatus[i] == 0){
                string uri = srcmanager.allSources[i]->uri;
                if(ping_ip_cam(uri)){
                    std::cout << "Camera Ping Successful. Adding it again to pipeline\n";

                    gchar pad_name[16] = { };
                    g_snprintf(pad_name, 15, "sink_%u", i);

                    GstPad *resinkpad, *srcpad;

                    GstElement *source_bin = create_source_bin (sourcebinCount, (char *)uri.c_str());
                    sourcebinCount++;
                    if (!source_bin) {
                        g_printerr ("Failed to create source bin. Exiting.\n");
                    }
                    gst_bin_add (GST_BIN (pipeline.pipeline), source_bin);
                    pipeline.sourcebins[i] = source_bin;

                    resinkpad = gst_element_get_request_pad (pipeline.streammux, pad_name);
                    if (!resinkpad) {
                        g_printerr ("Streammux request resink pad failed. Exiting.\n");
                    }
                    srcpad = gst_element_get_static_pad (source_bin, "src");
                    if (!srcpad) {
                        g_printerr ("Failed to get src pad of source bin. Exiting.\n");
                    } else
                        g_printerr ("Successfully get src pad of source bin.\n");

                    if (gst_pad_link (srcpad, resinkpad) != GST_PAD_LINK_OK) {
                        g_printerr ("Failed to link source bin to restream muxer. Exiting.\n");
                    } else 
                        g_printerr ("Successfully link source bin to restream muxer.\n");

                    gst_object_unref (srcpad);
                    gst_object_unref (resinkpad);

                    GstStateChangeReturn stateChange;
                    gst_element_set_state(pipeline.pipeline, GST_STATE_PAUSED);
                    stateChange = gst_element_set_state(source_bin, GST_STATE_PLAYING);
                    gst_element_set_state(pipeline.pipeline, GST_STATE_PLAYING);

                    bool sourcebinStatus = false;

                    switch (stateChange)
                    {
                        case GST_STATE_CHANGE_SUCCESS:
                            g_printerr("State change: Success");
                            srcmanager.allSourcesStatus[i] = 1;
                            break;
                        case GST_STATE_CHANGE_FAILURE:
                            g_printerr("State change: Failure");
                            srcmanager.allSourcesStatus[i] = 0;
                            break;
                        default:
                            break;
                    }
                } else
                    std::cout << "Camera Ping Unsuccessful.\n";
            } else if(srcmanager.allSourcesStatus[i] == 2){
                gchar pad_name[16] = { };
                GstPad *sinkpad;
                g_snprintf(pad_name, 15, "sink_%u", i);
                g_mutex_lock(&pipeline.Lock1);
                sinkpad = gst_element_get_static_pad(pipeline.streammux, pad_name);
                gst_element_release_request_pad(pipeline.streammux, sinkpad);
                gst_element_set_state(pipeline.sourcebins[i], GST_STATE_NULL);
                gst_bin_remove(GST_BIN (pipeline.pipeline), pipeline.sourcebins[i]);
                gst_element_set_state(pipeline.pipeline, GST_STATE_PLAYING);
                gst_object_unref(sinkpad);
                g_mutex_unlock(&pipeline.Lock1);
                std::cout<<"Removed stream data of "<<srcmanager.allSources[i]->uri<<std::endl;
                srcmanager.allSourcesStatus[i] = 0;
            }
        }
        sleep(5);
        }

    if(PipelineExecutor.joinable()){
        PipelineExecutor.join();
    }

    /* Out of the main loop, clean up nicely */
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline.pipeline, GST_STATE_NULL);
    g_print ("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline.pipeline));
    g_source_remove (pipeline.bus_watch_id);
    g_main_loop_unref (pipeline.loop);
    return 0;
}
