use std::io;
use std::io::Read;
use std::thread::sleep;
use std::time::Duration;

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

    let mut buffer: Vec<char> = vec![];
    loop {
        let mut buf = [0u8; 1];
        while let Err(_) = port.read_exact(&mut buf) {}

        if buf[0] as char == '\n' {
            // Handle line
            let to_split = buffer.iter().clone().collect::<String>();
            let commands = to_split.split(":").collect::<Vec<&str>>();
            if commands.len() > 1 {
                if commands[0].starts_with("FINISH_FLAG") {
                    println!("RESULT WITH {}", commands[1].trim());
                    return;
                }
            }
            println!("{}", buffer.into_iter().collect::<String>());
            buffer = vec![];
        } else {
            buffer.push(buf[0] as char);
        }
    }
}
