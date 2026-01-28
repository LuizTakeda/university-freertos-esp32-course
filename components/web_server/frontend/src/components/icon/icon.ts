import "./icon.scss";
import ArrowUp from "./svg/alt-arrow-up-svgrepo-com.svg";
import SortHorizontal from "./svg/square-sort-horizontal-svgrepo-com.svg";
import Tunning from "./svg/tuning-square-2-svgrepo-com.svg";

type Name = "arrow-up" | "sort-horizontal" | "tunning";

const ICON_MAP: Record<Name, string> = {
  "arrow-up": ArrowUp,
  "sort-horizontal": SortHorizontal,
  "tunning": Tunning,
};

export default class IconElement extends HTMLElement {
  static get observedAttributes() {
    return ["name"];
  }

  constructor() {
    super();
  }

  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    if (name === "name" && oldValue !== newValue) {
      this._render();
    }
  }

  connectedCallback() {
    this._render();
  }

  private _render() {
    const name = this.getAttribute("name") as Name;
    this.innerHTML = ICON_MAP[name] || "";
  }

  get name(): Name {
    return this.getAttribute("name") as Name;
  }

  set name(value: Name) {
    this.setAttribute("name", value);
  }
}

customElements.define("icon-element", IconElement);