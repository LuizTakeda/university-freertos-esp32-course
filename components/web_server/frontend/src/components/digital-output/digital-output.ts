import "./digital-output.scss";

type DigitalOutputState = "on" | "off" | "loading" | "warning";

export default class DigitalOutputElement extends HTMLElement {
  private _state: DigitalOutputState | null = null;

  constructor() {
    super();
  }

  connectedCallback() {
    this._loadValue();

    this.addEventListener("click", this._handleClick);
  }

  disconnectedCallback() {
    this.removeEventListener("click", this._handleClick);
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

  set state(newState: DigitalOutputState) {
    if (this._state === newState) {
      return;
    }

    this.classList.remove("on", "off", "loading", "warning");
    this.classList.add(newState);

    this._state = newState;
  }

  private _handleClick = async () => {
    switch (this.state) {
      case "on":
        await this._updateState(false);
        break;

      case "off":
        await this._updateState(true);
        break;

      case "warning":
        await this._loadValue();
        break;

      default:
    }
  }

  private async _loadValue() {
    this.state = "loading";

    try {
      const result = await fetch(`/api/digital-output?id=${this.num}`);

      if (!result.ok) {
        this.state = "warning";
        return;
      }

      const { state } = await result.json();

      this.state = state ? "on" : "off";
    } catch (error) {
      console.error(error);
      this.state = "warning";
    }
  }

  private async _updateState(newState: boolean) {
    this.state = "loading";

    try {
      const result = await fetch(`/api/digital-output?id=${this.num}`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify({ state: newState })
      });

      if (!result.ok) {
        this.state = "warning";
        return;
      }

      const { state } = await result.json();

      this.state = state ? "on" : "off";
    } catch (error) {
      console.error(error);
      this.state = "warning";
    }
  }
}

customElements.define("digital-output-element", DigitalOutputElement);