#ifndef RASLITEX_TIMER_H
#define RASLITEX_TIMER_H
namespace rlx {

    struct Timer
    {
        Timer()                           = default;
        Timer(const Timer&)               = delete;
        Timer& operator = (const Timer&)  = delete;

        static Timer& get();

        unsigned int  frameNumber;
        unsigned int  lastFrameTimestamp;// ����: 0.001 sec
        unsigned int  lastFrameTime;// ����: 0.001 sec

        //�����������
        bool   isPaused;
        double averageFrameTime;
        float  fps;

        static void update();
        static void init();
        static void deinit();
        // ����
        static unsigned getGlobalTime();
    private:
        static Timer* ms_singleton;
    };

}//ns timer
#endif // RASLITEX_TIMER_H


