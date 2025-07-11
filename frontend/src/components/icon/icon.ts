import "./icon.scss";
import Moon from "./svg/moon-svgrepo-com.svg";
import Sun from "./svg/sun-svgrepo-com.svg";
import ArrowUp from "./svg/alt-arrow-up-svgrepo-com.svg";

type Name = "" | "moon" | "sun" | "arrow-up";

export default class IconElement extends HTMLElement {
  private _name: Name;

  constructor() {
    super();

    this._name = (this.getAttribute("name") ?? "") as Name;
  }

  connectedCallback() {
    switch (this._name) {
      case "":
        break;

      case "moon":
        this.innerHTML = Moon;
        break;

      case "sun":
        this.innerHTML = Sun;
        break;

      case "arrow-up":
        this.innerHTML = ArrowUp;
        break

      default:
    }
  }

  set name(value: Name) { this._name = value }
  get name(): Name { return this._name ?? "" }
}

customElements.define("icon-element", IconElement);