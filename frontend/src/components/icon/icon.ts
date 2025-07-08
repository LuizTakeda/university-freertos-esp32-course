import "./icon.scss";
import Moon from "./svg/moon-svgrepo-com.svg";
import Sun from "./svg/sun-svgrepo-com.svg";
import ArrowUp from "./svg/alt-arrow-up-svgrepo-com.svg";
import Home from "./svg/home-svgrepo-com.svg";
import Cpu from "./svg/cpu-bolt-svgrepo-com.svg";
import Settings from "./svg/settings-svgrepo-com.svg";
import SortHorizontal from "./svg/square-sort-horizontal-svgrepo-com.svg";
import Tunning from "./svg/tuning-square-2-svgrepo-com.svg"

type Name = "" | "moon" | "sun" | "arrow-up" | "home" | "cpu" | "settings" | "sort-horizontal" | "tunning";

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

      case "home":
        this.innerHTML = Home;
        break

      case "cpu":
        this.innerHTML = Cpu;
        break

      case "settings":
        this.innerHTML = Settings;
        break

      case "sort-horizontal":
        this.innerHTML = SortHorizontal;
        break

      case "tunning":
        this.innerHTML = Tunning;
        break

      default:
    }
  }

  set name(value: Name) { this._name = value }
  get name(): Name { return this._name ?? "" }
}

customElements.define("icon-element", IconElement);