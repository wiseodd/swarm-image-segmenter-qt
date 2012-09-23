#ifndef SEGMENTER_ENGINE_H
#define SEGMENTER_ENGINE_H

#include "pso_cluster.h"

class SegmenterEngine
{
public:
    SegmenterEngine();
    GBest segmentImageHost(Data *datas, int data_size, int channel,
                           int particle_size, int cluster_size, int max_iter);
    GBest segmentImageDevice(Data *datas, int *flat_datas, int data_size,
                             int channel, int particle_size, int cluster_size,
                             int max_iter);
};

#endif // SEGMENTER_ENGINE_H
