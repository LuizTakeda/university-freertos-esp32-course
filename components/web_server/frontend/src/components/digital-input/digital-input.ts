import DeviceEvents, { NewInputStateEvent } from "../../utils/device-events";
import "./digital-input.scss";

type DigitalInputState = "on" | "off" | "loading" | "error";

export default class DigitalInputElement extends HTMLElement {
  private _state: DigitalInputState | null = null;

  constructor() {
    super();
  }

  connectedCallback() {
    this._loadValue();

    const deviceEvents = DeviceEvents.getInstance();

    deviceEvents.addEventListener("digital-input", this._handleNewStateEvent);
  }

  get num() {
    const num = this.getAttribute("num");

    if (num === null) {
      throw new Error("Num undefined");
    }

    return Number(num);
  }

  get state() {
    return this._state ?? "loading";
  }

  set state(newState: DigitalInputState) {
    if (this._state === newState) {
      return;
    }

    this.classList.remove("on", "off", "loading", "error");
    this.classList.add(newState);

    this._state = newState;
  }

  private async _loadValue() {
    this.state = "loading";

    try {
      const result = await fetch(`/api/digital-input/${this.num}`);

      if (!result.ok) {
        this.state = "error";
        return;
      }

      const { state } = await result.json();

      this.state = state ? "on" : "off";
    } catch (error) {
      console.error(error);
      this.state = "error";
    }
  }

  private _handleNewStateEvent = (data: NewInputStateEvent["data"]) => {
    if (this.num === data.num) {
      this.state = data.value === 1 ? "on" : "off";
    }
  }
}

customElements.define("digital-input-element", DigitalInputElement);