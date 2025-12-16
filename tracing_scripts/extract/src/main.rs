use std::fmt::Display;
use std::fs::File;
use std::io::Write;
use std::process::exit;
use std::thread::sleep;
use std::time::Duration;

use csv::Writer;
use types::parse::SerialEventDataIterator;

#[derive(Debug, Clone)]
struct Config {
    port: String,
    baud_rate: u32,
    output_file: String,
    task_mapping_file: String,
}

enum ArgState {
    Ready,
    ReadPort,
    ReadByteRate,
    ReadOutput,
    ReadMapping,
}

impl Default for Config {
    fn default() -> Self {
        Config {
            port: tokio_serial::available_ports().map_or_else(
                |_| "/dev/ttyUSB0".to_string(),
                |ports| {
                    ports.first().map_or_else(
                        || "/dev/ttyUSB0".to_string(),
                        |port| port.port_name.to_string(),
                    )
                },
            ),
            baud_rate: 115200,
            output_file: "./log_entries.csv".to_string(),
            task_mapping_file: "./mapping.csv".to_string(),
        }
    }
}

impl Display for Config {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(&format!(
            "Config {{ port: {}, baud_rate: {}, output: {}, task_mapping_file: {} }}",
            self.port, self.baud_rate, self.output_file, self.task_mapping_file
        ))
    }
}

fn main() {
    println!("[App] Reading config");

    let args = std::env::args();

    let (_, config) = args.fold(
        (ArgState::Ready, Config::default()),
        |(state, mut config), arg| match state {
            ArgState::Ready => (
                if arg == "-p" {
                    ArgState::ReadPort
                } else if arg == "-b" {
                    ArgState::ReadByteRate
                } else if arg == "-o" {
                    ArgState::ReadOutput
                } else if arg == "-m" {
                    ArgState::ReadMapping
                } else {
                    ArgState::Ready
                },
                config,
            ),
            ArgState::ReadPort => {
                config.port = arg.to_string();
                (ArgState::Ready, config)
            }
            ArgState::ReadByteRate => {
                config.baud_rate = arg.parse().unwrap_or(config.baud_rate);
                (ArgState::Ready, config)
            }
            ArgState::ReadOutput => {
                config.output_file = arg.to_string();
                (ArgState::Ready, config)
            }
            ArgState::ReadMapping => {
                config.task_mapping_file = arg.to_string();
                (ArgState::Ready, config)
            }
        },
    );

    println!("[App] Using this config for export: {}", config);
    println!("[App] Opening serial port!");

    let mut port = match tokio_serial::new(config.port, config.baud_rate).open() {
        Ok(port) => port,
        Err(_) => panic!("[App] Failed to open serial port! Are you sure a device is connected?"),
    };

    println!("[App] Opened serial port!");
    println!("[App] Resetting device!");

    port.write_request_to_send(true).unwrap();
    port.write_data_terminal_ready(false).unwrap();

    sleep(Duration::from_millis(100));

    port.write_request_to_send(false).unwrap();
    port.write_data_terminal_ready(false).unwrap();

    println!("[App] Start reading from device:");

    let mut writer = Writer::from_path(config.output_file)
        .expect("[App] Could not create output file! Do you have the right permissions?");

    let mut iterator = SerialEventDataIterator::new(port);
    (&mut iterator).for_each(|data| writer.serialize(data).unwrap());

    writer.flush().unwrap();

    let mut mapping_file = File::create(config.task_mapping_file)
        .expect("[App] Could not create task mapping file! Do you have the right permissions?");
    writeln!(&mut mapping_file, "taskid,task_name").unwrap();
    iterator
        .task_names()
        .iter()
        .for_each(|data| writeln!(&mut mapping_file, "{}", data).unwrap());

    if let Some(value) = iterator.return_value() {
        exit(value);
    } else {
        eprintln!("No return value found!");
    }
    exit(1);
}
