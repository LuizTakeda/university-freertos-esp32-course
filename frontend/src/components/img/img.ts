import "./img.scss";
import UTFPRLogo from "./svg/UTFPR_logo.svg"

type Name = "" | "utfpr";

export default class ImageElement extends HTMLElement {
  private _name: Name;

  constructor() {
    super();

    this._name = (this.getAttribute("name") ?? "") as Name;
  }

  connectedCallback() {
    switch (this._name) {
      case "utfpr":
        this.innerHTML = UTFPRLogo;
        break;

      default:
    }
  }

  set name(value: Name) { this._name = value }
  get name(): Name { return this._name ?? "" }
}

customElements.define("image-element", ImageElement);