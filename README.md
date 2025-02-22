# entry_point

Common entry point for web and native app.

## usage example

```
entry_point::execute_main_loop([](auto timeslice_in_seconds, auto cancel) {

  // exit update loop
  cancel();
});
```

## build

See `test/build_and_run` for native and `test/em_build_and_run` for web
