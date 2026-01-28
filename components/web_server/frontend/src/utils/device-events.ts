type BaseEvent<N extends string, D extends object> = {
  name: N;
  data: D;
};

export type NewInputStateEvent = BaseEvent<"digital-input", { num: number; value: number }>;
export type NewAnalogStateEvent = BaseEvent<"analog-input", { num: number; value: number }>;

type Events = NewInputStateEvent | NewAnalogStateEvent;

type EventData<N extends Events["name"]> = Extract<Events, { name: N }>["data"];

export default class DeviceEvents {
  private static instance: DeviceEvents;
  private eventSource: EventSource;
  
  private listeners = new Map<Function, (e: MessageEvent) => void>();

  private constructor() {
    this.eventSource = new EventSource("/api/events");
    
    this.eventSource.onerror = () => {
      console.error("EventSource: Connection failed. Retrying...");
    };
  }

  static getInstance(): DeviceEvents {
    if (!DeviceEvents.instance) {
      DeviceEvents.instance = new DeviceEvents();
    }
    return DeviceEvents.instance;
  }

  addEventListener<N extends Events["name"]>(
    event: N,
    callback: (data: EventData<N>) => void
  ): void {
    const internalCallback = ({ data }: MessageEvent) => {
      try {
        const parsedData = JSON.parse(data);
        callback(parsedData);
      } catch (e) {
        console.error(`[DeviceEvents] Parse fail for ${event}:`, e);
      }
    };

    this.listeners.set(callback, internalCallback);
    this.eventSource.addEventListener(event, internalCallback);
  }

  removeEventListener<N extends Events["name"]>(
    event: N,
    callback: (data: EventData<N>) => void
  ): void {
    const internalCallback = this.listeners.get(callback);
    if (internalCallback) {
      this.eventSource.removeEventListener(event, internalCallback);
      this.listeners.delete(callback);
    }
  }
}