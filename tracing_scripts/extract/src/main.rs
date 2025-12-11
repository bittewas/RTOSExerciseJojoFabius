use std::fmt::Display;
use std::fs::File;
use std::io::Read;
use std::io::Write;
use std::process::exit;
use std::thread::sleep;
use std::time::Duration;

use csv::Writer;
use types::GeneralEventData;
use types::QueueData;
use types::QueueEventType;
use types::TaskData;
use types::TaskEventType;
use types::TickData;
use types::TickEventType;

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

    sleep(Duration::from_millis(1000));

    port.write_request_to_send(false).unwrap();
    port.write_data_terminal_ready(false).unwrap();

    println!("[App] Start reading from device:");

    let mut writer = Writer::from_path(config.output_file)
        .expect("[App] Could not create output file! Do you have the right permissions?");
    let mut mapping_file = File::create(config.task_mapping_file)
        .expect("[App] Could not create task mapping file! Do you have the right permissions?");
    writeln!(&mut mapping_file, "taskid,task_name").unwrap();

    let mut buffer: Vec<char> = vec![];
    let return_value = loop {
        let mut buf = [0u8; 1];
        while port.read_exact(&mut buf).is_err() {}

        if buf[0] as char == '\n' {
            // Handle line
            let to_split = buffer.iter().clone().collect::<String>();
            let commands = to_split.split(":").collect::<Vec<&str>>();
            if commands.len() > 1 {
                let command = commands[0].split(" ").collect::<Vec<&str>>()[2];
                let value = commands[1].replace("[0m", "");
                match command {
                    "FINISH_FLAG" => {
                        println!("[Serial] RESULT WITH {}", value.trim());

                        break value
                            .trim()
                            .parse::<i32>()
                            .expect("Could not parse return value!");
                    }
                    "TASK_DEBUG" => match parse_task_line(value.trim()) {
                        Ok(data) => {
                            writer.serialize(data).unwrap();
                        }
                        Err(data) => {
                            if data != "Header file!" {
                                eprintln!("[App] [Error] {}", data)
                            }
                        }
                    },
                    "TICK_DEBUG" => {
                        if let Ok(data) = parse_tick_line(value.trim()) {
                            writer.serialize(data).unwrap();
                        }
                    }
                    "QUEUE_DEBUG" => {
                        if let Ok(data) = parse_queue_line(value.trim()) {
                            writer.serialize(data).unwrap();
                        }
                    }
                    "TASK_NAME" => {
                        writeln!(&mut mapping_file, "{}", value.trim().replace(";", ",")).unwrap();
                    }
                    _ => {
                        println!("[Serial] ({}) {}", command.trim(), value);
                    }
                }
                buffer = vec![];
            }
        } else if (buf[0] as char) != '\r' {
            buffer.push(buf[0] as char);
        }
    };
    writer.flush().unwrap();

    exit(return_value);
}

fn parse_queue_line(line: &str) -> Result<GeneralEventData, String> {
    let data = line.split(";").collect::<Vec<&str>>();

    if data.len() != 6 {
        return Err("Wrong format!".to_string());
    }

    if data[0].trim() == "Message Type" {
        return Err("Header file!".to_string());
    }

    let queue_data = QueueData {
        eventtype: QueueEventType::try_from(
            data[0].trim().parse::<u32>().map_err(|_| "".to_string())?,
        )?,
        queue: data[1].trim().parse().map_err(|_| "".to_string())?,
        tick: data[2].trim().parse().map_err(|_| "".to_string())?,
        timestamp: data[3].trim().parse().map_err(|_| "".to_string())?,
        taskid: data[4].trim().parse().map_err(|_| "".to_string())?,
        ticks_to_wait: data[5].trim().parse().map_err(|_| "".to_string())?,
    };

    Ok(GeneralEventData::from(queue_data))
}

fn parse_tick_line(line: &str) -> Result<GeneralEventData, String> {
    let data = line.split(";").collect::<Vec<&str>>();

    if data.len() != 4 {
        return Err("Wrong format!".to_string());
    }

    if data[0].trim() == "C Time" {
        return Err("Header file!".to_string());
    }

    let queue_data = TickData {
        eventtype: TickEventType::IncrementTick,
        tick: data[0].trim().parse().map_err(|_| "".to_string())?,
        timestamp: data[1].trim().parse().map_err(|_| "".to_string())?,
        new_tick_time: data[2].trim().parse().map_err(|_| "".to_string())?,
        taskid: data[3].trim().parse().map_err(|_| "".to_string())?,
    };

    Ok(GeneralEventData::from(queue_data))
}

fn parse_task_line(line: &str) -> Result<GeneralEventData, String> {
    let data = line.split(";").collect::<Vec<&str>>();

    if data.len() != 6 {
        return Err("Wrong format!".to_string());
    }

    if data[0].trim() == "Message Type" {
        return Err("Header file!".to_string());
    }

    let task_data =
        TaskData {
            eventtype: TaskEventType::try_from(data[0].trim().parse::<u32>().map_err(|err| {
                format!("(Task) Failed to parse eventtype. Reason: {}", err).to_string()
            })?)?,
            tick: data[1].trim().parse().map_err(|err| {
                format!("(Task) Failed to parse tick. Reason: {}", err).to_string()
            })?,
            timestamp: data[2].trim().parse().map_err(|err| {
                format!("(Task) Failed to parse timestamp. Reason: {}", err).to_string()
            })?,
            taskid: data[3].trim().parse().map_err(|err| {
                format!("(Task) Failed to parse taskid. Reason: {}", err).to_string()
            })?,
            affected_task_id: data[4].trim().parse().map_err(|err| {
                format!("(Task) Failed to parse affected task id. Reason: {}", err).to_string()
            })?,
            delay: data[5].trim().parse().map_err(|err| {
                format!("(Task) Failed to parse delay. Reason: {}", err).to_string()
            })?,
        };

    Ok(GeneralEventData::from(task_data))
}
