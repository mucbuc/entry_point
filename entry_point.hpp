#pragma once

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#include <thread>
#endif
#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <tuple>
#include <utility>

namespace entry_point {
const unsigned default_fps_native = 60;

template <typename T>
void execute_main_loop(T update_callback, unsigned fps = 0)
{
    using namespace std::chrono;

    time_point<system_clock> frame_begin { system_clock::now() };

#ifdef __EMSCRIPTEN__

#ifndef NO_ENTRY
    bool simulate_infinite_loop = true;
#else
    bool simulate_infinite_loop = false;
#endif

    using data_type = std::tuple<T, time_point<system_clock>>;
    data_type* data = new data_type(std::move(update_callback), frame_begin);
    emscripten_set_main_loop_arg(
        [](void* userData) {
            data_type* data(reinterpret_cast<data_type*>(userData));
            auto& update_callback { std::get<0>(*data) };
            auto& frame_begin { std::get<1>(*data) };
            const auto now = system_clock::now();
            const duration<double> diff(now - frame_begin);
            frame_begin = now;

            update_callback(diff.count(), [data]() {
                delete data;
                emscripten_cancel_main_loop();
            });
        },
        data,
        fps,
        simulate_infinite_loop);
#else

    fps = fps ? fps : default_fps_native;
    duration<double> time_slice_target { 1.0 / fps };
    bool done = false;
    while (!done) {

        auto work_begin { system_clock::now() };
        const duration<double> diff(work_begin - frame_begin);

        if (diff.count()) {
            const auto correction = (diff * fps - duration<double>(1)) / fps;
            time_slice_target -= correction;
        }

        frame_begin = work_begin;
        update_callback(diff.count(), [&done]() {
            done = true;
        });

        const auto work_end { system_clock::now() };
        const auto sleep_duration { time_slice_target - (work_end - work_begin) };
        if (sleep_duration > duration<double>(0)) {
            std::this_thread::sleep_for(sleep_duration);
        }
    }
#endif
}

template <typename State_Type, typename... T>
void execute_main_loop(State_Type& state, unsigned fps, std::tuple<T...> modules)
{
    using ModulesType = std::tuple<T...>;
    static constexpr size_t ModulesSize = std::tuple_size<ModulesType>::value;

    std::array<std::function<void()>, ModulesSize> release_callbacks;
    bool all_modules_initialized = false;

#ifndef __EMSCRIPTEN__
    const std::thread::id this_thread_id = std::this_thread::get_id();
    auto on_initialized = [&all_modules_initialized, &release_callbacks, init_counter = 0, this_thread_id](auto done) mutable {
        ASSERT(this_thread_id == std::this_thread::get_id());
#else
    auto on_initialized = [&all_modules_initialized, &release_callbacks, init_counter = 0](auto done) mutable {
#endif
        release_callbacks[init_counter] = done;
        ++init_counter;

        if (init_counter == ModulesSize) {
            all_modules_initialized = true;
        }
    };

    auto init_mod = [&on_initialized](auto&& mod) {
        mod.init(on_initialized);
    };

    std::apply([&init_mod](auto&&... mod_instance) {
        ((init_mod(mod_instance)), ...);
    },
        modules);

    entry_point::execute_main_loop([&all_modules_initialized,
                                       &state,
                                       &release_callbacks,
                                       &modules
#ifndef NDEBUG
                                       ,
                                       init_wait_frames = 0
#endif
    ](auto ft, auto cancel) mutable {
        if (all_modules_initialized) {

            bool canceled = false;
            auto release_and_cancel = [cancel,
                                          &release_callbacks,
                                          &canceled]() {
                if (!canceled) {
                    auto release_mod = [](auto&& mod) {
                        ASSERT(mod);

                        mod();
                    };

                    std::apply([release_mod](auto&&... mod_instance) {
                        ((
                             release_mod(mod_instance)),
                            ...);
                    },
                        release_callbacks);

                    cancel();
                    canceled = true;
                }
            };

            std::apply([&state, ft, release_and_cancel](auto&&... mod_instance) {
                ((
                     mod_instance.run(state, ft, release_and_cancel)),
                    ...);
            },
                modules);
        }
#ifndef NDEBUG
        else {
            ASSERT(++init_wait_frames < 30);
        }
#endif
    },
        fps);
}

} // entry_point
