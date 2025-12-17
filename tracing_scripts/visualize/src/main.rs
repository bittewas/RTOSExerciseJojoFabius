use std::{collections::HashMap, io, path::PathBuf, thread::sleep, time::Duration};

use clap::Parser;
use egui::{Align2, Color32, Response, Stroke, Ui, Vec2};
use egui_extras::{Size, StripBuilder};
use egui_plot::{LineStyle, Plot, PlotPoint, Points, Polygon, Text};
use palette::{rgb::Rgb, FromColor};
use rfd::FileDialog;
use tokio_serial::SerialPortInfo;
use types::{parse::SerialEventDataIterator, GeneralEventData};

#[derive(Parser, Debug)]
#[command(name = "Viewer")]
struct Args {
    #[arg(short, long, value_name = "FILE", default_value = "./log_entries.csv")]
    input: PathBuf,
}

fn main() -> io::Result<()> {
    let args = Args::parse();
    let events = read_events(&args.input)?;

    let task_segment_data = get_task_segments(&events);

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_decorations(false)
            .with_resizable(true)
            .with_inner_size((1200.0, 800.0)),
        ..Default::default()
    };
    eframe::run_native(
        "Task Schedule Diagram",
        options,
        Box::new(|_cc| {
            Ok(Box::new(TaskScheduleApp {
                task_segment_data,
                input_file: args.input,
                ports: tokio_serial::available_ports().unwrap_or_default(),
                selected_port: None,
            }))
        }),
    )
    .unwrap();
    Ok(())
}

fn read_events(csv_path: &PathBuf) -> io::Result<Vec<GeneralEventData>> {
    let reader = csv::ReaderBuilder::new()
        .has_headers(true)
        .from_path(csv_path)?;

    Ok(reader
        .into_deserialize()
        .filter_map(|map| map.ok())
        .collect())
}

type TaskSegmentData = (
    HashMap<u32, Vec<GeneralEventData>>,
    Vec<(u32, String)>,
    Vec<u32>,
);

fn get_task_segments(data: &[GeneralEventData]) -> TaskSegmentData {
    let mut queue_ids: Vec<u32> = vec![];
    let task_events: Vec<&GeneralEventData> = data.iter().collect();

    let mut map: HashMap<u32, Vec<GeneralEventData>> = HashMap::new();

    task_events.into_iter().for_each(|entry| {
        let event_data = map.entry(entry.taskid).or_default();
        event_data.push(entry.clone());
        if entry.is_queue_event() && !queue_ids.contains(&entry.affected_object) {
            queue_ids.push(entry.affected_object);
        }
    });

    map.iter_mut()
        .for_each(|(_, vec)| vec.sort_by_key(|t| t.timestamp));

    let mut task_ids: Vec<(u32, String)> = vec![];

    map.iter().for_each(|(task_id, data)| {
        task_ids.push((
            *task_id,
            data.first()
                .map(|data| data.task_name.to_string())
                .unwrap_or("".to_string()),
        ));
    });

    task_ids.sort_unstable_by_key(|(id, _)| *id);
    (map, task_ids, queue_ids)
}

struct TaskScheduleApp {
    task_segment_data: TaskSegmentData,
    input_file: PathBuf,
    ports: Vec<SerialPortInfo>,
    selected_port: Option<SerialPortInfo>,
}

fn task_box<'t>(
    name: impl Into<String>,
    start: f64,
    end: f64,
    height: f64,
    color: Color32,
) -> Polygon<'t> {
    let name = name.into();
    Polygon::new(
        name.clone(),
        vec![
            [start, height - 0.25],
            [start, height + 0.25],
            [end, height + 0.25],
            [end, height - 0.25],
            [start, height - 0.25],
        ],
    )
    .fill_color(color)
    .stroke(Stroke::new(1.0, color))
    .style(LineStyle::Solid)
    .allow_hover(false)
    .name(name)
    .highlight(false)
}

impl TaskScheduleApp {
    fn display_controls(&mut self, ui: &mut Ui) -> Response {
        ui.horizontal(|ui| {
                    ui.group(|ui| {
                        ui.vertical(|ui| {
                            StripBuilder::new(ui)
                                .size(Size::exact(160.0))
                                .size(Size::exact(40.0))
                                .horizontal(|mut strip| {
                                    strip.cell(|ui| {
                                        let label = egui::Label::new(format!(
                                            "{}",
                                            self.input_file.display()
                                        ))
                                        .truncate();
                                        ui.add(label);
                                    });
                                    strip.cell(|ui| {
                                        if ui.button("Find").clicked() {
                                            if let Some(path) = FileDialog::new()
                                                .add_filter("CSV", &["csv", "CSV"])
                                                .pick_file()
                                            {
                                                self.input_file = path;

                                                let events = read_events(&self.input_file).unwrap();

                                                self.task_segment_data = get_task_segments(&events);
                                            }
                                        }
                                    });
                                });

                            ui.horizontal_centered(|ui| {
                                let button = egui::Button::new("Reload from file")
                                    .min_size(Vec2::new(200.0, 0.0));
                                if ui.add(button).clicked() {
                                    let events = read_events(&self.input_file).unwrap();

                                    self.task_segment_data = get_task_segments(&events);
                                }
                            });
                        });
                    });

                    ui.group(|ui| {
                        StripBuilder::new(ui)
                            .size(Size::exact(200.0))
                            .horizontal(|mut strip| {
                                strip.cell(|ui| {
                                    StripBuilder::new(ui)
                                        .size(Size::exact(10.0))
                                        .size(Size::exact(10.0))
                                        .vertical(|mut strip| {
                                            strip.cell(|ui| {
                                                StripBuilder::new(ui)
                                                    .size(Size::exact(70.0))
                                                    .size(Size::exact(180.0))
                                                    .horizontal(|mut strip| {
                                                        strip.cell(|ui| {
                                                            ui.label("Serial Port:");
                                                        });
                                                        strip.cell(|ui| {
                                                            egui::ComboBox::from_id_salt(
                                                                "serial-port",
                                                            )
                                                            .selected_text(format!(
                                                                "{:?}",
                                                                if let Some(port) =
                                                                    &self.selected_port
                                                                {
                                                                    port.clone().port_name
                                                                } else {
                                                                    "None".to_string()
                                                                }
                                                            ))
                                                            .width(180.0)
                                                            .show_ui(ui, |ui| {
                                                                for port in &self.ports {
                                                                    ui.selectable_value(
                                                                        &mut self.selected_port,
                                                                        Some(port.clone()),
                                                                        port.clone().port_name,
                                                                    );
                                                                }
                                                            });
                                                        });
                                                    });
                                            });
                                            strip.cell(|ui| {
                                                StripBuilder::new(ui)
                                                    .size(Size::exact(130.0))
                                                    .size(Size::exact(120.0))
                                                    .horizontal(|mut strip| {
                                                        strip.cell(|ui| {
                                                            if ui
                                                            .add(
                                                                egui::Button::new(
                                                                    "Reload serial devices",
                                                                )
                                                                .min_size(Vec2::new(130.0, 0.0)),
                                                            )
                                                            .clicked()
                                                        {
                                                            self.ports =
                                                                tokio_serial::available_ports()
                                                                    .unwrap_or_default();
                                                        }
                                                        });
                                                        strip.cell(|ui| {
                                                            if ui.add(
                                                                egui::Button::new(
                                                                    "Reload from serial",
                                                                )
                                                                .min_size(Vec2::new(120.0, 0.0)),
                                                            )
                                                            .clicked()
                                                            {
                                                                if let Some(port) = self.selected_port.clone() {
                                                                    let mut port = match tokio_serial::new(port.port_name, 115200).open() {
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

                                                                    let mut iterator = SerialEventDataIterator::new(port);
                                                                    let events = (&mut iterator).collect::<Vec<GeneralEventData>>();

                                                                    self.task_segment_data = get_task_segments(&events);
                                                                }
                                                            }
                                                        });
                                                    });
                                            });
                                        });
                                });
                            });
                    });
                }).response
    }

    fn display_plot(&mut self, ui: &mut Ui) -> Response {
        let plot = Plot::new("schedule_plot")
            .allow_zoom(true)
            .allow_drag(true)
            .allow_scroll(true)
            .show_grid(true)
            .x_axis_label("Tick Count")
            .y_axis_label("Task ID")
            .label_formatter(|x, y| format!("({0:.2}, {1:.2}) {2}", y.x, y.y, x));

        plot.show(ui, |plot_ui| {
            let task_ids = &self.task_segment_data.1;
            let queue_ids = &self.task_segment_data.2;
            for (idx, (task_id, name)) in task_ids.iter().enumerate() {
                let y_pos = idx as f64 + 0.5;
                let color = color_for_task(idx as u32, task_ids.len());

                let mut data = self.task_segment_data.0.get(task_id).unwrap().clone();
                data.sort_by_key(|data| data.tick);

                let mut last_in_data = None;

                data.iter()
                    .for_each(|event_data| match event_data.eventtype.as_str() {
                        "traceTASK_SWITCHED_IN" => {
                            last_in_data = Some((event_data.timestamp, event_data.tick));
                        }
                        "traceTASK_SWITCHED_OUT" => {
                            if let Some((_, tick)) = last_in_data {
                                plot_ui.add(task_box(
                                    "",
                                    tick as f64,
                                    event_data.tick as f64,
                                    y_pos,
                                    color,
                                ));
                                last_in_data = None
                            }
                        }
                        _ => {}
                    });

                if let Some((_, tick)) = last_in_data {
                    plot_ui.add(task_box(
                        "",
                        tick as f64,
                        data.iter().map(|t| t.tick).max().unwrap_or(tick) as f64,
                        y_pos,
                        color,
                    ));
                }

                data.iter()
                    .for_each(|event_data| match event_data.eventtype.as_str() {
                        "traceTASK_DELAY_UNTIL" => {
                            plot_ui.points(
                                Points::new(
                                    "DELAY UNTIL",
                                    vec![[event_data.delay as f64, y_pos - 0.25]],
                                )
                                .filled(true)
                                .radius(4.0)
                                .shape(egui_plot::MarkerShape::Up)
                                .color(Color32::WHITE),
                            );
                        }
                        "traceTASK_DELAY" => {
                            plot_ui.points(
                                Points::new("DELAY", vec![[event_data.delay as f64, y_pos - 0.25]])
                                    .filled(true)
                                    .radius(4.0)
                                    .shape(egui_plot::MarkerShape::Up)
                                    .color(Color32::PURPLE),
                            );
                        }
                        "traceQUEUE_RECEIVE" => {
                            plot_ui.points(
                                Points::new(
                                    "QUEUE RECEIVE",
                                    vec![[event_data.tick as f64, y_pos - 0.125]],
                                )
                                .filled(true)
                                .radius(4.0)
                                .shape(egui_plot::MarkerShape::Circle)
                                .color(color_for_task(
                                    queue_ids
                                        .iter()
                                        .position(|&o| o == event_data.affected_object)
                                        .unwrap() as u32,
                                    queue_ids.len(),
                                )),
                            );
                        }
                        "traceQUEUE_SEND" => {
                            plot_ui.points(
                                Points::new(
                                    "QUEUE SEND",
                                    vec![[event_data.tick as f64, y_pos + 0.125]],
                                )
                                .filled(true)
                                .radius(4.0)
                                .shape(egui_plot::MarkerShape::Diamond)
                                .color(color_for_task(
                                    queue_ids
                                        .iter()
                                        .position(|&o| o == event_data.affected_object)
                                        .unwrap() as u32,
                                    queue_ids.len(),
                                )),
                            );
                        }
                        "traceTASK_CREATE" => {
                            plot_ui.points(
                                Points::new(
                                    format!("Created Task {}", event_data.affected_object),
                                    vec![[event_data.tick as f64, y_pos]],
                                )
                                .filled(true)
                                .radius(4.0)
                                .shape(egui_plot::MarkerShape::Plus)
                                .color(Color32::GREEN),
                            );
                            if let Some(pos) = task_ids
                                .iter()
                                .position(|(task, _)| *task == event_data.affected_object)
                            {
                                plot_ui.points(
                                    Points::new(
                                        format!("Task created by {}", event_data.task_name),
                                        [event_data.tick as f64, pos as f64 + 0.5],
                                    )
                                    .filled(true)
                                    .radius(4.0)
                                    .shape(egui_plot::MarkerShape::Plus)
                                    .color(Color32::PURPLE),
                                );
                            }
                        }
                        "traceTASK_DELETE" => {
                            plot_ui.points(
                                Points::new(
                                    format!("Delete Task {}", event_data.affected_object),
                                    vec![[event_data.tick as f64, y_pos]],
                                )
                                .filled(true)
                                .radius(4.0)
                                .shape(egui_plot::MarkerShape::Cross)
                                .color(Color32::GREEN),
                            );
                            if let Some(pos) = task_ids
                                .iter()
                                .position(|(task, _)| *task == event_data.affected_object)
                            {
                                plot_ui.points(
                                    Points::new(
                                        format!("Task deleted by {}", event_data.task_name),
                                        [event_data.tick as f64, pos as f64 + 0.5],
                                    )
                                    .filled(true)
                                    .radius(4.0)
                                    .shape(egui_plot::MarkerShape::Cross)
                                    .color(Color32::PURPLE),
                                );
                            }
                        }
                        _ => {}
                    });

                plot_ui.text(
                    Text::new(name, PlotPoint::new(-10.0, y_pos), name.to_string())
                        .anchor(Align2::RIGHT_CENTER),
                );
            }
        })
        .response
    }
}

impl eframe::App for TaskScheduleApp {
    fn update(&mut self, ctx: &egui::Context, _: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.vertical_centered(|ui| ui.heading("Task Schedule Diagram"));

            ui.add_space(8.0);

            ui.vertical(|ui| {
                self.display_controls(ui);

                ui.separator();

                self.display_plot(ui);
            });
        });
    }
}

fn color_for_task(task_index: u32, total: usize) -> Color32 {
    let hue = (task_index as f32 / total as f32) * 360.0;
    let hsl = palette::Hsl::new(palette::RgbHue::from_degrees(hue), 0.5, 0.6);
    let rgb: Rgb = Rgb::from_color(hsl);
    // Convert to egui::Color32
    Color32::from_rgba_unmultiplied(
        (rgb.red * 255.0) as u8,
        (rgb.green * 255.0) as u8,
        (rgb.blue * 255.0) as u8,
        255,
    )
}
