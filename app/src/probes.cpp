#include "probes.h"
#include <string.h>
#include <thread>

extern sourceManager srcmanager;


void checkInactiveSource(vector<int> source_ids){

    for(int i = 0; i < source_ids.size(); i++){
        srcmanager.allSources[source_ids[i]]->consecutiveMissedFrames = 0;
        srcmanager.allSources[source_ids[i]]->firstFrame = 1;
    }
    for (int i = 0; i < srcmanager.allSources.size(); i++){
        if (srcmanager.allSources[i]->firstFrame){
            srcmanager.allSources[i]->consecutiveMissedFrames++;
        }
        if (srcmanager.allSources[i]->consecutiveMissedFrames > 80){
            if (srcmanager.allSourcesStatus[i] == 1){
                cout << "Got End of Stream from: " << srcmanager.allSources[i]->uri<<endl;
                srcmanager.allSourcesStatus[i] = 2;
                srcmanager.allSources[i]->firstFrame = 0;
            } else
                srcmanager.allSources[i]->consecutiveMissedFrames = 0;
        }
    }
}


/* tiler_sink_pad_buffer_probe  will extract metadata received on OSD sink pad
* and update params for drawing rectangle, object information etc. */
GstPadProbeReturn
tiler_sink_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info,
                           gpointer u_data) {
    GstBuffer *buf = (GstBuffer *) info->data;
    guint num_rects = 0;
    NvDsObjectMeta *obj_meta = NULL;
    guint vehicle_count = 0;
    guint person_count = 0;
    NvDsMetaList *l_frame = NULL;
    NvDsMetaList *l_obj = NULL;
    NvDsDisplayMeta *display_meta = NULL;
    Point *polygon = NULL; //a pointer to the polygon
    int polygonSides = 0;
    SourceProperties *source = NULL;

    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    vector<int> batch_meta_frame_ids;
    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL;
         l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);
        batch_meta_frame_ids.push_back(frame_meta->source_id);
        source = srcmanager.allSources[frame_meta->source_id];
        polygon = source->polygon;
        polygonSides = source->polygon_sides;
        display_meta = nvds_acquire_display_meta_from_pool(
                batch_meta);
        NvOSD_LineParams *line_params = display_meta->line_params;
        int n = srcmanager.allSources[frame_meta->source_id]->polygon_sides;

        for (int i = 0; i < polygonSides; i++) {
            int next = (i + 1) % n;
            line_params[i].x1 = polygon[i].x;
            line_params[i].y1 = polygon[i].y;
            line_params[i].x2 = polygon[next].x;
            line_params[i].y2 = polygon[next].y;
            line_params[i].line_width = 4;
            line_params[i].line_color = (NvOSD_ColorParams) {0.0, 0.0, 1.0, 1.0};
            display_meta->num_lines++;
        }

        /* char const  *str = "hola";
         sprintf(display_meta->text_params->display_text, "%s", str);
         display_meta->num_labels++;*/

        /*   gchar *msg = NULL;
           g_object_get(G_OBJECT (u_data), "last-message", &msg, NULL);*/
        nvds_add_display_meta_to_frame(frame_meta, display_meta);
    }
    checkInactiveSource(batch_meta_frame_ids);
    return GST_PAD_PROBE_OK;
}