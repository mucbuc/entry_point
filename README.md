  # entry_point

A single-header main loop abstraction for native and Emscripten targets.

## Overview

`entry_point` provides two overloads of `execute_main_loop` — one for raw frame callbacks, and one that manages the full lifecycle of a tuple of plugin modules.

---

## `execute_main_loop` — Raw Callback

```cpp
entry_point::execute_main_loop([](float ft, auto cancel) {
    // called every frame
    // ft: time since last frame in seconds
    // cancel(): stops the loop
}, fps);
```

`fps` is optional and defaults to 60 on native. On Emscripten it is passed directly to `emscripten_set_main_loop_arg` and can be set to `0` to use the browser's `requestAnimationFrame` rate.

On native the loop runs on the calling thread and blocks until `cancel()` is called. On Emscripten the loop is handed off to the browser and the call returns immediately.

---

## `execute_main_loop` — Module Lifecycle

```cpp
entry_point::execute_main_loop(state, fps, std::make_tuple(
    MyModule(),
    AnotherModule()
));
```

Manages initialization, the frame loop, and teardown for a tuple of modules. Each module must implement:

```cpp
const char* name();

void init(std::function<void(std::function<void()>)> done);

template<typename State, typename Cancel>
void run(State& state, float ft, Cancel cancel);
```

### Lifecycle

1. All modules are initialized concurrently by calling `init` on each in tuple order.
2. Each module calls `done` when ready, passing its own teardown callback.
3. Once all modules have called `done`, the frame loop begins.
4. Each frame, `run` is called on every module in tuple order.
5. Any module can stop the loop by calling `cancel()` from within `run`. This triggers teardown of all modules in tuple order before the loop exits.

### Module Example

```cpp
struct MyModule {
    const char* name() { return "MyModule"; }

    void init(std::function<void(std::function<void()>)> done) {
        // setup
        done([]() {
            // teardown
        });
    }

    template<typename State, typename Cancel>
    void run(State& state, float ft, Cancel cancel) {
        // called every frame
    }
};
```

---

## Notes

- Designed for single-threaded use. Modules may manage their own threads internally if needed.
- `cancel()` is idempotent — calling it more than once from different modules in the same frame is safe.
- On Emscripten, `emscripten_set_main_loop_arg` is called once and cannot be called again for the lifetime of the page. Calling `execute_main_loop` more than once on Emscripten is undefined behavior.
- In debug builds (`NDEBUG` not defined), an assertion fires if modules take more than 30 frames to complete initialization, catching modules that never call their `done` callback.
