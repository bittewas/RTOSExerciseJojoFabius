use serde::{Deserialize, Serialize};

pub mod parse;

#[derive(Serialize, Deserialize, Debug, Clone, Copy)]
pub enum TaskEventType {
    #[serde(rename = "traceTASK_CREATE")]
    Create = 0,
    #[serde(rename = "traceTASK_CREATE_FAILED")]
    CreateFailed = 1,
    #[serde(rename = "traceTASK_DELETE")]
    Delete = 2,
    #[serde(rename = "traceTASK_DELAY")]
    Delay = 3,
    #[serde(rename = "traceTASK_DELAY_UNTIL")]
    DelayUntil = 4,
    #[serde(rename = "traceTASK_SWITCHED_IN")]
    SwitchedIn = 5,
    #[serde(rename = "traceTASK_SWITCHED_OUT")]
    SwitchedOut = 6,
}

impl TryFrom<u32> for TaskEventType {
    type Error = String;

    fn try_from(value: u32) -> Result<Self, Self::Error> {
        Ok(match value {
            0 => TaskEventType::Create,
            1 => TaskEventType::CreateFailed,
            2 => TaskEventType::Delete,
            3 => TaskEventType::Delay,
            4 => TaskEventType::DelayUntil,
            5 => TaskEventType::SwitchedIn,
            6 => TaskEventType::SwitchedOut,
            _ => return Err("".to_string()),
        })
    }
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct TaskData {
    pub eventtype: TaskEventType,
    pub tick: u32,
    pub timestamp: u32,
    pub taskid: u32,
    pub affected_task_id: u32,
    pub delay: u32,
    pub task_name: String,
}

#[derive(Serialize, Deserialize, Debug, Clone, Copy)]
pub enum QueueEventType {
    #[serde(rename = "traceQUEUE_RECEIVE")]
    Recieve = 0,
    #[serde(rename = "traceQUEUE_RECEIVE_FAILED")]
    RecieveFailed = 1,
    #[serde(rename = "traceQUEUE_RECEIVE_FROM_ISR")]
    RecieveFromISR = 2,
    #[serde(rename = "traceQUEUE_RECEIVE_FROM_ISR_FAILED")]
    RecieveFromISRFailed = 3,
    #[serde(rename = "traceQUEUE_SEND")]
    Send = 4,
    #[serde(rename = "traceQUEUE_SEND_FAILED")]
    SendFailed = 5,
    #[serde(rename = "traceQUEUE_SEND_FROM_ISR")]
    SendFromISR = 6,
    #[serde(rename = "traceQUEUE_SEND_FROM_ISR_FAILED")]
    SendFromISRFailed = 7,
    #[serde(rename = "traceQUEUE_SET_SEND")]
    SetSend = 8,
}

impl TryFrom<u32> for QueueEventType {
    type Error = String;

    fn try_from(value: u32) -> Result<Self, Self::Error> {
        Ok(match value {
            0 => QueueEventType::Recieve,
            1 => QueueEventType::RecieveFailed,
            2 => QueueEventType::RecieveFromISR,
            3 => QueueEventType::RecieveFromISRFailed,
            4 => QueueEventType::Send,
            5 => QueueEventType::SendFailed,
            6 => QueueEventType::SendFromISR,
            7 => QueueEventType::SendFromISRFailed,
            8 => QueueEventType::SetSend,
            _ => return Err("".to_string()),
        })
    }
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct QueueData {
    pub eventtype: QueueEventType,
    pub queue: u32,
    pub tick: u32,
    pub timestamp: u32,
    pub taskid: u32,
    pub ticks_to_wait: u32,
    pub task_name: String,
}

#[derive(Serialize, Deserialize, Debug, Clone, Copy)]
pub enum TickEventType {
    #[serde(rename = "traceTASK_INCREMENT_TICK")]
    IncrementTick,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct TickData {
    pub eventtype: TickEventType,
    pub tick: u32,
    pub timestamp: u32,
    pub new_tick_time: u32,
    pub taskid: u32,
    pub task_name: String,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct GeneralEventData {
    pub eventtype: String,
    pub tick: u32,
    pub timestamp: u32,
    pub taskid: u32,
    pub affected_object: u32,
    pub delay: u32,
    pub task_name: String,
}

impl GeneralEventData {
    pub fn is_queue_event(&self) -> bool {
        matches!(
            self.eventtype.as_str(),
            "traceQUEUE_SEND"
                | "traceQUEUE_SEND_FAILED"
                | "traceQUEUE_SEND_FROM_ISR"
                | "traceQUEUE_SEND_FROM_ISR_FAILED"
                | "traceQUEUE_RECEIVE"
                | "traceQUEUE_RECEIVE_FAILED"
                | "traceQUEUE_RECEIVE_FROM_ISR"
                | "traceQUEUE_RECEIVE_FROM_ISR_FAILED"
        )
    }
}

impl From<TickData> for GeneralEventData {
    fn from(value: TickData) -> Self {
        Self {
            eventtype: serde_json::to_string(&value.eventtype)
                .unwrap()
                .replace("\"", ""),
            tick: value.tick,
            timestamp: value.timestamp,
            taskid: value.taskid,
            affected_object: value.new_tick_time,
            delay: value.new_tick_time - value.tick,
            task_name: value.task_name,
        }
    }
}

impl From<QueueData> for GeneralEventData {
    fn from(value: QueueData) -> Self {
        Self {
            eventtype: serde_json::to_string(&value.eventtype)
                .unwrap()
                .replace("\"", ""),
            tick: value.tick,
            timestamp: value.timestamp,
            taskid: value.taskid,
            affected_object: value.queue,
            delay: value.ticks_to_wait,
            task_name: value.task_name,
        }
    }
}

impl From<TaskData> for GeneralEventData {
    fn from(value: TaskData) -> Self {
        Self {
            eventtype: serde_json::to_string(&value.eventtype)
                .unwrap()
                .replace("\"", ""),
            tick: value.tick,
            timestamp: value.timestamp,
            taskid: value.taskid,
            affected_object: value.affected_task_id,
            delay: value.delay,
            task_name: value.task_name,
        }
    }
}

#[test]
pub fn test_tick_casting() {
    let tick_data = TickData {
        eventtype: TickEventType::IncrementTick,
        tick: 100,
        timestamp: 1000,
        taskid: 1307,
        new_tick_time: 101,
        task_name: "Blas".to_string(),
    };

    let general_event_data = GeneralEventData::from(tick_data.clone());

    assert_eq!(general_event_data.eventtype, "traceTASK_INCREMENT_TICK");
    assert_eq!(general_event_data.tick, tick_data.tick);
    assert_eq!(general_event_data.taskid, tick_data.taskid);
    assert_eq!(general_event_data.task_name, "Blas".to_string());
}

#[test]
pub fn test_task_casting() {
    let task_data = TaskData {
        eventtype: TaskEventType::Create,
        tick: 100,
        timestamp: 100,
        taskid: 1307,
        affected_task_id: 1308,
        delay: 0,
        task_name: "Blos".to_string(),
    };

    let general_event_data = GeneralEventData::from(task_data.clone());

    assert_eq!(general_event_data.eventtype, "traceTASK_CREATE");
    assert_eq!(general_event_data.tick, task_data.tick);
    assert_eq!(general_event_data.taskid, task_data.taskid);
}

#[test]
pub fn test_queue_casting() {
    let queue_data = QueueData {
        eventtype: QueueEventType::Recieve,
        queue: 50,
        tick: 100,
        timestamp: 1000,
        taskid: 1307,
        ticks_to_wait: 0,
        task_name: "TESTING".to_string(),
    };

    let general_event_data = GeneralEventData::from(queue_data.clone());

    assert_eq!(general_event_data.eventtype, "traceQUEUE_RECEIVE");
    assert_eq!(general_event_data.tick, queue_data.tick);
    assert_eq!(general_event_data.taskid, queue_data.taskid);
}
