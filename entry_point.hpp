#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#include <thread>
#endif
#include <chrono>

namespace entry_point
{
    const unsigned default_fps_native = 60;

    template<typename T>
    void execute_main_loop(T logic, unsigned fps = 0)
    {
        using namespace std::chrono;

        time_point<system_clock> frame_begin { system_clock::now() };

#ifdef __EMSCRIPTEN__
        using data_type = std::tuple<T, time_point<system_clock>>;
        data_type data { std::make_tuple(logic, frame_begin) };
        emscripten_set_main_loop_arg(
            [](void* userData) {
                data_type * data(reinterpret_cast<data_type *>(userData));
                auto logic { std::get<0>(* data) };
                auto & frame_begin { std::get<1>(* data) };
                const auto now = system_clock::now();
                const duration<double> diff(now - frame_begin);
                frame_begin = now;

                logic(diff.count(), [](){
                    emscripten_cancel_main_loop();
                });
            },
            & data,
            fps,
            true);
#else

        fps = fps ? fps : default_fps_native;
        duration<double> time_slice_target { 1.0 / fps };
        bool done = false;
        while (!done) {

            auto work_begin { system_clock::now() };
            const duration<double> diff( work_begin - frame_begin );

            if (diff.count()) {
                const auto correction = (diff * fps - duration<double>(1)) / fps;
                time_slice_target -= correction;
            }

            frame_begin = work_begin;
            logic(diff.count(), [& done](){
                done = true;
            });

            const auto work_end { system_clock::now() };
            const auto sleep_duration { time_slice_target - (work_end - work_begin) };
            std::this_thread::sleep_for(sleep_duration);
        }
#endif
    }
}
