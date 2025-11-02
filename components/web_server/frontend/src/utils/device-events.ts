type Event<N extends string, D extends object> = {
  name: N,
  data: D
}

export type NewInputStateEvent = Event<"digital-input", { num: number, value: number }>

type Events = NewInputStateEvent

type GetData<E extends Events, N extends E['name']> =
  E extends { name: N, data: infer D } ? D : never;

export default class DeviceEvents {
  private static instance: DeviceEvents;
  private eventSource: EventSource;

  private constructor() {
    this.eventSource = new EventSource("/api/events");
  }

  static getInstance() {
    if (!DeviceEvents.instance) {
      DeviceEvents.instance = new DeviceEvents();
    }
    return DeviceEvents.instance;
  }

  addEventListener<E extends Events["name"]>(
    event: E, callback: (data: GetData<Events, E>) => void
  ) {
    this.eventSource.addEventListener(event, ({ data }) => {
      try {
        callback(JSON.parse(data));
      } catch (_) {
        console.error("Event: Parse fail");
      }
    });
  }
}