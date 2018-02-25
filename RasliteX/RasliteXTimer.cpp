#include "RasliteXTimer.h"
#include <windows.h>
#include <mmsystem.h>

namespace rlx {
    static bool   s_queryPerfCountEnabled;
    static double s_queryPerfCounterFreq;

    unsigned _system_time()
    {
        if(s_queryPerfCountEnabled)
        {
            static LONGLONG qpcMillisPerTick;
            ::QueryPerformanceCounter((LARGE_INTEGER*)&qpcMillisPerTick);
            return (unsigned)(qpcMillisPerTick * s_queryPerfCounterFreq);
        }
        else
            return unsigned(::timeGetTime());
    }
    static void _init_timing()
    {
        LONGLONG time;
        s_queryPerfCountEnabled = (QueryPerformanceFrequency((LARGE_INTEGER*)&time) > 0);
        if(s_queryPerfCountEnabled)
            s_queryPerfCounterFreq = 1000.0 / time;
    }
    //////////////////////////////////////////////////////////////
    Timer* Timer::ms_singleton = nullptr;

    unsigned Timer::getGlobalTime()
    {
        return _system_time();
    }
   
    Timer& Timer::get()
    {
        return (Timer&)*ms_singleton;
    }
    void Timer::update()
    {
        if(!ms_singleton) 
            return;
        if(!ms_singleton->isPaused)
            ms_singleton->frameNumber++;

        unsigned thisTime = _system_time();
        ms_singleton->lastFrameTime      = thisTime - ms_singleton->lastFrameTimestamp;
        ms_singleton->lastFrameTimestamp = thisTime;

        if(ms_singleton->frameNumber > 1) 
        {
            if(ms_singleton->averageFrameTime <= 0)
                ms_singleton->averageFrameTime = (double)ms_singleton->lastFrameTime;
            else
            {
                ms_singleton->averageFrameTime *= 0.99;
                ms_singleton->averageFrameTime += 0.01 * (double)ms_singleton->lastFrameTime;
                ms_singleton->fps = (float)(1000.0 / ms_singleton->averageFrameTime);
            }
        }
    }
    void Timer::init()
    {
        _init_timing();
        if(!ms_singleton)
            ms_singleton = new Timer();
        ms_singleton->frameNumber = 0;

        ms_singleton->lastFrameTimestamp =  _system_time();
        ms_singleton->lastFrameTime = 0;

        ms_singleton->isPaused = false;

        ms_singleton->averageFrameTime = 0;
        ms_singleton->fps = 0;
    }
    void Timer::deinit()
    {
        delete ms_singleton;
        ms_singleton = NULL;
    }

}//ns zlp