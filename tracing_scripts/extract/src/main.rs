use std::io::Read;
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

fn main() {
    println!("Connecting to serial console!");

    for port in tokio_serial::available_ports().unwrap() {
        println!("Port: {:?}", port);
    }

    let mut port = match tokio_serial::new("/dev/ttyUSB0", 115200).open() {
        Ok(port) => port,
        Err(_) => panic!("Failed to open serial port!"),
    };

    println!("Opened serial port!");

    port.write_request_to_send(true).unwrap();
    port.write_data_terminal_ready(false).unwrap();

    sleep(Duration::from_millis(1000));

    port.write_request_to_send(false).unwrap();
    port.write_data_terminal_ready(false).unwrap();

    let mut writer = Writer::from_path("./output.csv").expect("Could not create file");

    let mut buffer: Vec<char> = vec![];
    loop {
        let mut buf = [0u8; 1];
        while port.read_exact(&mut buf).is_err() {}

        if buf[0] as char == '\n' {
            // Handle line
            let to_split = buffer.iter().clone().collect::<String>();
            let commands = to_split.split(":").collect::<Vec<&str>>();
            if commands.len() > 1 {
                let command = commands[0].split(" ").collect::<Vec<&str>>()[2];
                let value = commands[1];
                match command {
                    "FINISH_FLAG" => {
                        println!("RESULT WITH {}", value.trim());
                        break;
                    }
                    "TASK_DEBUG" => match parse_task_line(value.trim()) {
                        Ok(data) => {
                            print!("Writing data");
                            writer.serialize(data).unwrap();
                        }
                        Err(err) => {
                            println!("{}", err);
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
                    _ => {
                        println!("MISSED: {} {}", command, value);
                    }
                }
                buffer = vec![];
            }
        } else if (buf[0] as char) != '\r' {
            buffer.push(buf[0] as char);
        }
    }
    writer.flush().unwrap();
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
        tick: data[2].trim().parse().map_err(|_| "".to_string())?,
        taskid: data[4].trim().parse().map_err(|_| "".to_string())?,
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

    let queue_data = TaskData {
        eventtype: TaskEventType::try_from(
            data[0].trim().parse::<u32>().map_err(|_| "1".to_string())?,
        )?,
        tick: data[2].trim().parse().map_err(|_| "2".to_string())?,
        taskid: data[4].trim().parse().map_err(|_| "3".to_string())?,
    };

    Ok(GeneralEventData::from(queue_data))
}
