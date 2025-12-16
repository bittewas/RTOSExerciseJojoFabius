use tokio_serial::SerialPort;

use crate::{
    GeneralEventData, QueueData, QueueEventType, TaskData, TaskEventType, TickData, TickEventType,
};

pub struct SerialEventDataIterator {
    return_value: Option<i32>,
    task_names: Vec<String>,
    port: Box<dyn SerialPort>,
}

impl SerialEventDataIterator {
    pub fn new(port: Box<dyn SerialPort>) -> SerialEventDataIterator {
        SerialEventDataIterator {
            return_value: None,
            task_names: vec![],
            port,
        }
    }

    pub fn return_value(&self) -> Option<i32> {
        self.return_value
    }

    pub fn task_names(&self) -> &[String] {
        &self.task_names
    }
}

impl Iterator for SerialEventDataIterator {
    type Item = GeneralEventData;

    fn next(&mut self) -> Option<Self::Item> {
        if self.return_value.is_some() {
            return None;
        }

        let mut buffer: Vec<char> = vec![];
        loop {
            let mut buf = [0u8; 1];
            while self.port.read_exact(&mut buf).is_err() {}

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

                            self.return_value = Some(
                                value
                                    .trim()
                                    .parse::<i32>()
                                    .expect("Could not parse return value!"),
                            );
                            break None;
                        }
                        "TASK_DEBUG" => match parse_task_line(value.trim()) {
                            Ok(data) => {
                                break Some(data);
                            }
                            Err(data) => {
                                if data != "Header file!" {
                                    eprintln!("[App] [Error] {}", data)
                                }
                            }
                        },
                        "TICK_DEBUG" => {
                            if let Ok(data) = parse_tick_line(value.trim()) {
                                break Some(data);
                            }
                        }
                        "QUEUE_DEBUG" => {
                            if let Ok(data) = parse_queue_line(value.trim()) {
                                break Some(data);
                            }
                        }
                        "TASK_NAME" => {
                            self.task_names.push(value.trim().replace(";", ","));
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
        }
    }
}

pub fn parse_queue_line(line: &str) -> Result<GeneralEventData, String> {
    let data = line.split(";").collect::<Vec<&str>>();

    if data.len() != 7 {
        return Err("Wrong format!".to_string());
    }

    if data[0].trim() == "Message Type" {
        return Err("Header file!".to_string());
    }

    let queue_data =
        QueueData {
            eventtype: QueueEventType::try_from(data[0].trim().parse::<u32>().map_err(|err| {
                format!("(Queue) Failed to parse eventtype. Reason: {}", err).to_string()
            })?)?,
            queue: data[1].trim().parse().map_err(|err| {
                format!("(Queue) Failed to parse queue. Reason: {}", err).to_string()
            })?,
            tick: data[2].trim().parse().map_err(|err| {
                format!("(Queue) Failed to parse tick. Reason: {}", err).to_string()
            })?,
            timestamp: data[3].trim().parse().map_err(|err| {
                format!("(Queue) Failed to parse timestamp. Reason: {}", err).to_string()
            })?,
            taskid: data[4].trim().parse().map_err(|err| {
                format!("(Queue) Failed to parse taskid. Reason: {}", err).to_string()
            })?,
            ticks_to_wait: data[5].trim().parse().map_err(|err| {
                format!("(Queue) Failed to parse ticks_to_wait. Reason: {}", err).to_string()
            })?,
            task_name: data[6].trim().to_string(),
        };

    Ok(GeneralEventData::from(queue_data))
}

pub fn parse_tick_line(line: &str) -> Result<GeneralEventData, String> {
    let data = line.split(";").collect::<Vec<&str>>();

    if data.len() != 5 {
        return Err("Wrong format!".to_string());
    }

    if data[0].trim() == "C Time" {
        return Err("Header file!".to_string());
    }

    let queue_data =
        TickData {
            eventtype: TickEventType::IncrementTick,
            tick: data[0].trim().parse().map_err(|err| {
                format!("(Tick) Failed to parse tick. Reason: {}", err).to_string()
            })?,
            timestamp: data[1].trim().parse().map_err(|err| {
                format!("(Tick) Failed to parse timestamp. Reason: {}", err).to_string()
            })?,
            new_tick_time: data[2].trim().parse().map_err(|err| {
                format!("(Tick) Failed to parse new_tick_time. Reason: {}", err).to_string()
            })?,
            taskid: data[3].trim().parse().map_err(|err| {
                format!("(Tick) Failed to parse taskid. Reason: {}", err).to_string()
            })?,
            task_name: data[4].trim().to_string(),
        };

    Ok(GeneralEventData::from(queue_data))
}

pub fn parse_task_line(line: &str) -> Result<GeneralEventData, String> {
    let data = line.split(";").collect::<Vec<&str>>();

    if data.len() != 7 {
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
            task_name: data[6].trim().to_string(),
        };

    Ok(GeneralEventData::from(task_data))
}
