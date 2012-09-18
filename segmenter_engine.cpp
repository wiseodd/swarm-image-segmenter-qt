#include "segmenter_engine.h"

SegmenterEngine::SegmenterEngine()
{

}

GBest SegmenterEngine::segmentImageHost(Data *datas, int data_size,
                                        int particle_size, int cluster_size,
                                        int max_iter)
{
    return hostPsoClustering(datas, data_size, particle_size, cluster_size,
                             max_iter);
}

GBest SegmenterEngine::segmentImageDevice(Data *datas, int *flat_datas,
                                          int data_size, int particle_size,
                                          int cluster_size, int max_iter)
{
    return devicePsoClustering(datas, flat_datas, data_size, particle_size,
                        cluster_size, max_iter);
}
