import "./analog-input.scss"
import DeviceEvents, { NewAnalogStateEvent } from "../../utils/device-events";

export default class AnalogInputElement extends HTMLElement {
  private path: SVGPathElement | null = null;

  private readonly MARGIN_LEFT = 30;
  private readonly MARGIN_RIGHT = 190;
  private readonly STEPS = 5;

  private records: number[] = [];

  constructor() {
    super();
  }

  connectedCallback() {
    const svgNS = "http://www.w3.org/2000/svg";

    const svg = document.createElementNS(svgNS, "svg");
    svg.setAttribute("viewBox", "0 0 200 100");
    svg.setAttribute("width", "100%");
    svg.setAttribute("height", "auto");

    this.path = document.createElementNS(svgNS, "path");
    this.path.setAttribute("fill", "none");
    this.path.setAttribute("stroke", "var(--color-utfpr)");
    this.path.setAttribute("stroke-width", "1");

    const lineX = document.createElementNS(svgNS, "line");
    lineX.setAttribute("x1", "30");
    lineX.setAttribute("y1", "90");
    lineX.setAttribute("x2", "190");
    lineX.setAttribute("y2", "90");
    lineX.setAttribute("stroke", "var(--color-text)");
    lineX.setAttribute("stroke-width", "1");

    const lineY = document.createElementNS(svgNS, "line");
    lineY.setAttribute("x1", "30");
    lineY.setAttribute("y1", "10");
    lineY.setAttribute("x2", "30");
    lineY.setAttribute("y2", "90");
    lineY.setAttribute("stroke", "var(--color-text)");
    lineY.setAttribute("stroke-width", "1");

    const maxText = document.createElementNS(svgNS, "text");
    maxText.setAttribute("x", "28");
    maxText.setAttribute("y", "10");
    maxText.setAttribute("font-size", "8");
    maxText.setAttribute("fill", "var(--color-text)");
    maxText.setAttribute("text-anchor", "end");
    maxText.setAttribute("dominant-baseline", "text-before-edge");
    maxText.innerHTML = String(this.max);

    const minText = document.createElementNS(svgNS, "text");
    minText.setAttribute("x", "28");
    minText.setAttribute("y", "90");
    minText.setAttribute("font-size", "8");
    minText.setAttribute("fill", "var(--color-text)");
    minText.setAttribute("text-anchor", "end");
    minText.setAttribute("dominant-baseline", "text-after-edge");
    minText.innerHTML = String(this.min);

    const currentText = document.createElementNS(svgNS, "text");
    currentText.setAttribute("x", "190");
    currentText.setAttribute("y", "10");
    currentText.setAttribute("font-size", "7");
    currentText.setAttribute("fill", "var(--color-text)");
    currentText.setAttribute("text-anchor", "end");
    currentText.setAttribute("dominant-baseline", "text-before-edge");

    const titleText = document.createElementNS(svgNS, "text");
    titleText.setAttribute("x", "100");
    titleText.setAttribute("y", "0");
    titleText.setAttribute("font-size", "7");
    titleText.setAttribute("fill", "var(--color-text)");
    titleText.setAttribute("text-anchor", "middle");
    titleText.setAttribute("dominant-baseline", "text-before-edge");
    titleText.innerHTML = this.innerHTML;
    this.innerHTML = "";

    svg.append(this.path, lineX, lineY, maxText, minText, currentText, titleText);

    this.append(svg);

    const deviceEvents = DeviceEvents.getInstance();

    deviceEvents.addEventListener("analog-input", this._handleNewStateEvent)
  }

  get name() {
    const name = this.getAttribute("name");

    if (name === null) {
      throw new Error("Name undefined");
    }

    return name;
  }

  get min() {
    const min = this.getAttribute("min");

    if (min === null) {
      throw new Error("Min undefined");
    }

    return Number(min);
  }

  get max() {
    const max = this.getAttribute("max");

    if (max === null) {
      throw new Error("Max undefined");
    }

    return Number(max);
  }

  get num() {
    const num = this.getAttribute("num");

    if (num === null) {
      throw new Error("Num undefined");
    }

    return Number(num);
  }

  addRecord(value: number) {
    if (this.path === null) {
      return;
    };
    const maxRecords = (this.MARGIN_RIGHT - this.MARGIN_LEFT) / this.STEPS;
    this.records = [value, ...this.records.slice(0, maxRecords)];

    let linePath = "M ";
    this.records.forEach((record, i) => {
      linePath += `${this.MARGIN_RIGHT - (i * this.STEPS)} ${record} `;
    });

    this.path.setAttribute("d", linePath);
  }

  private _handleNewStateEvent = (data: NewAnalogStateEvent["data"]) => {
    if (this.num === data.num) {
      this.addRecord(data.value);
    }
  }
}

customElements.define("analog-input-element", AnalogInputElement);