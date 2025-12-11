use std::{collections::HashMap, io, path::PathBuf};

use clap::Parser;
use egui::Color32;
use egui_plot::Plot;
use palette::{rgb::Rgb, FromColor, IntoColor, RgbHue};
use types::GeneralEventData;

#[derive(Parser, Debug)]
#[command(name = "Viewer")]
struct Args {
    #[arg(short, long, value_name = "FILE")]
    input: PathBuf,
}

fn main() -> io::Result<()> {
    let args = Args::parse();
    let events = read_events(&args.input)?;

    let (task_segments, max_tick, task_ids) = get_task_segments(&events);

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

type TaskSegmentData = (HashMap<u32, Vec<(u32, u32)>>, u32, Vec<u32>);

fn get_task_segments(data: &[GeneralEventData]) -> TaskSegmentData {
    let task_events: Vec<&GeneralEventData> = data
        .iter()
        .filter(|ev| {
            ev.eventtype == "traceTASK_SWITCHED_IN" || ev.eventtype == "traceTASK_SWITCHED_OUT"
        })
        .collect();

    let mut map: HashMap<u32, Vec<(u32, String)>> = HashMap::new();

    task_events.iter().for_each(|entry| {
        map.entry(entry.taskid)
            .or_default()
            .push((entry.tick, entry.eventtype.to_owned()));
    });

    map.iter_mut()
        .for_each(|vec| vec.1.sort_by_key(|&(t, _)| t));

    let mut max_ticks: u32 = 0;
    let mut task_ids: Vec<u32> = vec![];

    let task_segments = map
        .iter()
        .map(|(&task_id, vec)| {
            task_ids.push(task_id);

            let mut in_ticks: Vec<u32> = vec![];
            let mut out_ticks: Vec<u32> = vec![];

            vec.iter().for_each(|(tick, event)| match event.as_str() {
                "traceTASK_SWITCHED_IN" => in_ticks.push(*tick),
                "traceTASK_SWITCHED_OUT" => out_ticks.push(*tick),
                _ => {}
            });

            let mut segments: Vec<(u32, u32)> = vec![];
            let mut in_i = 0;
            let mut out_i = 0;

            while in_i < in_ticks.len() {
                let start = in_ticks[in_i];
                while out_i < out_ticks.len() && out_ticks[out_i] <= start {
                    out_i += 1;
                }
                if out_i < out_ticks.len() {
                    let end = out_ticks[out_i];
                    segments.push((start, end));
                    in_i += 1;
                    out_i += 1;
                } else {
                    // No matching OUT: use the latest tick from the whole file
                    let last_known_tick = data.iter().map(|e| e.tick).max().unwrap_or(start);
                    if last_known_tick > start {
                        segments.push((start, last_known_tick));
                    }
                    break;
                }
            }

            if !segments.is_empty() {
                max_ticks = max_ticks.max(segments.iter().map(|&(_, e)| e).max().unwrap_or(0));
                (task_id, segments)
            } else {
                let last_tick = data.iter().map(|e| e.tick).max().unwrap_or(0);
                max_ticks = max_ticks.max(last_tick);
                (task_id, vec![])
            }
        })
        .collect();

    task_ids.sort_unstable();
    (task_segments, max_ticks, task_ids)
}

fn calculate_x_ticks(max_tick: u32) -> Vec<f64> {
    if max_tick == 0 {
        return (0..10).map(|x| x as f64).collect();
    }
    let mut tick_step = (max_tick as f64 / 15.0).round() as u32;
    if tick_step == 0 {
        tick_step = 1;
    }
    if tick_step > 1 {
        let power_of_ten = 10u32.pow((tick_step as f64).log10().floor() as u32);
        let relative_step = tick_step as f64 / power_of_ten as f64;
        let nice_step = if relative_step < 1.5 {
            1
        } else if relative_step < 3.0 {
            2
        } else if relative_step < 7.0 {
            5
        } else {
            10
        };
        tick_step = nice_step * power_of_ten;
    }
    let mut ticks: Vec<f64> = Vec::new();
    let mut cur: f64 = 0.0;
    while cur <= max_tick as f64 {
        ticks.push(cur);
        cur += tick_step as f64;
    }
    ticks
}

struct TaskScheduleApp {
    task_segments: HashMap<u32, Vec<(u32, u32)>>,
    max_tick: u32,
    task_ids: Vec<u32>,
}

impl eframe::App for TaskScheduleApp {
    fn update(&mut self, ctx: &egui::Context, _: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            // Title
            ui.heading("Task Schedule Diagram");

            // A tiny bit of padding
            ui.add_space(8.0);

            // Build the Plot
            let plot = Plot::new("schedule_plot")
                .view_aspect(1.5) // width / height ratio
                .allow_zoom(true)
                .allow_drag(true)
                .allow_scroll(true)
                .show_grid(true)
                .x_axis_label("Tick Count")
                .y_axis_label("Task ID")
                .time_range(self.max_tick as f64)
                .domain(0.0, self.max_tick as f64 * 1.02)
                .label_formatter(|x, y, _| {
                    // Pretty y labels: task id numbers
                    format!("{}{}", "", y as u32)
                });

            // In egui_plot, there is no direct “BarSeries” that accepts horizontal bars,
            // but we can create a `BarPlot` where the “x” axis is the start tick and the
            // “y” axis is the *task index*.  We then set the `height` to the bar’s width.
            // The trick is to store points as `(start_tick, task_index, bar_width)`.

            let mut bar_plots: Vec<BarPlot> = Vec::new();

            for (idx, &task_id) in self.task_ids.iter().enumerate() {
                // Y position: we’ll just use the integer index (0,1,2, …)
                let y_pos = idx as f64 + 0.5; // +0.5 to center the bar

                let segments = self.task_segments.get(&task_id).unwrap();
                for &(start, end) in segments {
                    let width = (end - start) as f64;
                    // Each bar is represented as a single point whose height is the width.
                    let point = egui_plot::PlotPoint::new(start as f64, y_pos, width);
                    let mut bar = BarPlot::new(&format!("t{}-{}", task_id, start))
                        .points(vec![point])
                        .color(color_for_task(idx as u32, self.task_ids.len()))
                        .label(String::new()); // no label on each bar
                    // The height is treated as the bar's *value* on the Y‑axis
                    // – which is what `width` represents.
                    bar_plots.push(bar);
                }
            }

            // Now show the plot
            plot.show(ui, |plot_ui| {
                // Add the horizontal grid lines
                let ticks_x = calculate_x_ticks(self.max_tick);
                plot_ui.set_x_ticks(&ticks_x);

                // Add the Y‑tick labels – we need a custom handler because the
                // default y‑labels are just numbers.  We’ll use a “PlotLine” with
                // `visible=false` to inject the labels.
                for (idx, &task_id) in self.task_ids.iter().enumerate() {
                    // Draw a horizontal invisible line so egui_plot shows a label
                    let y = idx as f64 + 0.5;
                    let line = PlotLine::new(&format!("label_{}", task_id))
                        .points(vec![
                            egui_plot::PlotPoint::new(0.0, y, 0.0),
                            egui_plot::PlotPoint::new(0.0, y, 0.0),
                        ])
                        .visible(false)
                        .label(format!("{}", task_id));
                    plot_ui.line(line);
}

fn color_for_task(task_index: u32, total: usize) -> Color32 {
    // A simple hue rotation: hue ∈ [0, 360)
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
