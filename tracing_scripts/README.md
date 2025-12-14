# Export & Visualisation

This part of the project takes care of the visualisation and export of the tracing data.

To correctly debug the actual version you first need to flash the version to use for debugging to your clock.

## Export tracing data

To get the tracing data from the watch you can simply run `cargo run --bin extract` in this folder.

Our script will automatically use the first serial device it can find and try to talk to it.
If your setup has multiple devices and we did not choose the correct one automatically you can provide it using command flags

```sh
cargo run --bin extract -- -p serial_port_to_use -b baud_rate -o output_file_location -m task_name_mapping_file_location
```

### Interpreting Result value

Our extraction script will use the tracing data result to determine it's own result value and will provide it to std::out as well.
So if you don't find any errors in your log please note the following to determine which buffer overflowed while obtaining the tracing data.

| Bitmask | Buffer that overflowed |
| ------- | ---------------------- |
| 0x01    | Queue Message Buffer   |
| 0x02    | Tick Message Buffer    |
| 0x04    | Task Message Buffer    |

Afterwards you may increase the tracing buffer size or decrease the tracing time to prevent this overflow from happening.

Note: The export program may also crash on it's own if provided with wrong permissions/filenames and data. So please check the logs as well!

## Visualisation tracing script (python provided one)

This is the provided visualization script with our additions to obtain some more feature.

To run it first install the requirements listed in [requirements.txt](./requirements.txt).
Afterwards you can run the script using `python visualization.py`.
You may add the `-c` flag to use CPU cycle count instead of tick count as time value.

For the former we added visualization of queue send and receive events. The first is indicated by a bar with a down arrow with a dot instead of a head and the later by a up arrow with a dot instead of a head. Different queues are displayed in different colors.

Additionally we provide task delay markers by displaying the delay value with a black up arrow, when the task should continue execution.

As some of our tasks had execution times smaller than a tick before being switched out again we modified the visualization such that even those tasks are displayed for a full tick instead of not being displayed. As this is not ideal we also provide the possibility to simply switch out the tick with the CPU cycle count. This can be done by providing `-c` as argument to the script.

## Visualisation tracing script (Rust self build one)

TODO!
