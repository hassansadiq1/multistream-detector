#ifndef PROBES_H
#define PROBES_H

#include "gstnvdsmeta.h"
#include "context.h"

void checkInactiveSource(vector<int> source_ids);

 GstPadProbeReturn tiler_src_pad_buffer_probe(GstPad* , GstPadProbeInfo* , void* );

 GstPadProbeReturn osd_sink_pad_buffer_probe(GstPad* , GstPadProbeInfo* , void* );

#endif