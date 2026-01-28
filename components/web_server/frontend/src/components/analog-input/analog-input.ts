import "./analog-input.scss";
import DeviceEvents, { NewAnalogStateEvent } from "../../utils/device-events";

export default class AnalogInputElement extends HTMLElement {
  private path: SVGPathElement | null = null;
  private currentText: Element | null = null;
  private records: number[] = [];

  private readonly MARGIN_LEFT = 30;
  private readonly MARGIN_RIGHT = 190;
  private readonly STEPS = 5;
  private readonly VIEW_HEIGHT = 100;
  private readonly GRAPH_BOTTOM = 90;
  private readonly GRAPH_TOP = 10;

  constructor() {
    super();
  }

  connectedCallback() {
    this._setupGraph();
    DeviceEvents.getInstance().addEventListener("analog-input", this._handleNewStateEvent);
  }

  disconnectedCallback() {
    DeviceEvents.getInstance().removeEventListener("analog-input", this._handleNewStateEvent);
  }

  private _setupGraph() {
    const svgNS = "http://www.w3.org/2000/svg";
    const title = this.innerText;
    this.innerHTML = "";

    const svg = document.createElementNS(svgNS, "svg");
    svg.setAttribute("viewBox", `0 0 200 ${this.VIEW_HEIGHT}`);
    svg.setAttribute("width", "100%");
    svg.setAttribute("height", "100%");

    this.path = document.createElementNS(svgNS, "path");
    this.path.setAttribute("fill", "none");
    this.path.setAttribute("stroke", "var(--color-utfpr)");
    this.path.setAttribute("stroke-width", "1.5");

    const axes = document.createElementNS(svgNS, "g");
    axes.setAttribute("stroke", "var(--color-text)");
    axes.setAttribute("stroke-width", "1");
    axes.setAttribute("opacity", "0.5");

    const lineX = this._createSvgElement(svgNS, "line", { x1: "30", y1: "90", x2: "190", y2: "90" });
    const lineY = this._createSvgElement(svgNS, "line", { x1: "30", y1: "10", x2: "30", y2: "90" });
    axes.append(lineX, lineY);

    const minTxt = this._createText(svgNS, "28", "90", String(this.min), "end", "text-after-edge");
    const maxTxt = this._createText(svgNS, "28", "10", String(this.max), "end", "text-before-edge");

    this.currentText = this._createText(svgNS, "190", "10", "--", "end", "text-before-edge");

    const titleTxt = this._createText(svgNS, "100", "0", title, "middle", "text-before-edge");

    svg.append(axes, this.path, minTxt, maxTxt, this.currentText, titleTxt);
    this.appendChild(svg);
  }

  private _createSvgElement(ns: string, tag: string, attrs: Record<string, string>) {
    const el = document.createElementNS(ns, tag);
    for (const key in attrs) el.setAttribute(key, attrs[key]);
    return el;
  }

  private _createText(ns: string, x: string, y: string, content: string, anchor: string, baseline: string) {
    const txt = document.createElementNS(ns, "text");
    txt.setAttribute("x", x); txt.setAttribute("y", y);
    txt.setAttribute("font-size", "7");
    txt.setAttribute("fill", "var(--color-text)");
    txt.setAttribute("text-anchor", anchor);
    txt.setAttribute("dominant-baseline", baseline);
    txt.textContent = content;
    return txt;
  }

  addRecord(value: number) {
    if (!this.path || !this.currentText) return;

    this.currentText.textContent = value.toString();

    const range = this.max - this.min;
    const availableHeight = this.GRAPH_BOTTOM - this.GRAPH_TOP;
    const normalizedY = this.GRAPH_BOTTOM - (((value - this.min) / range) * availableHeight);

    const maxRecords = (this.MARGIN_RIGHT - this.MARGIN_LEFT) / this.STEPS;
    this.records = [normalizedY, ...this.records.slice(0, maxRecords)];

    const linePath = this.records.map((y, i) =>
      `${i === 0 ? 'M' : 'L'} ${this.MARGIN_RIGHT - (i * this.STEPS)} ${y}`
    ).join(" ");

    this.path.setAttribute("d", linePath);
  }

  get min() { return Number(this.getAttribute("min") || 0); }
  get max() { return Number(this.getAttribute("max") || 100); }
  get num() { return Number(this.getAttribute("num")); }

  private _handleNewStateEvent = (data: NewAnalogStateEvent["data"]) => {
    if (this.num === data.num) this.addRecord(data.value);
  }
}

customElements.define("analog-input-element", AnalogInputElement);