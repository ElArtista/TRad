#include "mainloop.h"
#include <prof.h>
#include <tinycthread.h>

typedef unsigned long timepoint_type;

timepoint_type clock_msec()
{
    return millisecs();
}

static inline void mainloop_perf(struct mainloop_data* loop_data, timepoint_type render_time)
{
    loop_data->perf.samples[loop_data->perf.cur_sample_cnt++] = render_time;
    if (loop_data->perf.cur_sample_cnt >= ML_PERF_SAMPLES) {
        float avgms = 0.0f;
        for (unsigned int i = 0; i < ML_PERF_SAMPLES; ++i)
            avgms += loop_data->perf.samples[i];
        avgms /= ML_PERF_SAMPLES;
        loop_data->perf.callback(loop_data->userdata, avgms, 1000.0f / avgms);
        loop_data->perf.cur_sample_cnt = 0;
    }
}

void mainloop(struct mainloop_data* loop_data)
{
    timepoint_type next_update = clock_msec();
    const float skip_ticks = 1000 / loop_data->updates_per_second;

    int loops;
    float interpolation;

    while (!loop_data->should_terminate) {
        loops = 0;
        while (clock_msec() > next_update && loops < loop_data->max_frameskip) {
            loop_data->update_callback(loop_data->userdata, skip_ticks);
            next_update += skip_ticks;
            ++loops;
        }

        timepoint_type rt = clock_msec();
        interpolation = (rt + skip_ticks - next_update) / skip_ticks;
        loop_data->render_callback(loop_data->userdata, interpolation);
        if (loop_data->perf.callback)
            mainloop_perf(loop_data, clock_msec() - rt);
    }
}

void mainloop_const(struct mainloop_data* loop_data)
{
    const float skip_ticks = 1000 / loop_data->updates_per_second;
    while (!loop_data->should_terminate) {
        timepoint_t t1 = clock_msec();
        loop_data->update_callback(loop_data->userdata, skip_ticks);
        loop_data->render_callback(loop_data->userdata, 1.0f);
        timepoint_t t2 = clock_msec();
        unsigned long long remaining = skip_ticks - (t2 - t1);
        if (remaining > 0) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 1000000 * remaining;
            thrd_sleep(&ts, 0);
        }
        if (loop_data->perf.callback)
            mainloop_perf(loop_data, (clock_msec() - t1));
    }
}
