import IconElement from "../icon/icon";
import DashboardOptionElement from "./dashboard-option";

DashboardOptionElement;

export type OptionClickEvent = CustomEvent<{ target: string }>;

declare global {
  interface HTMLElementEventMap {
    "option-click": OptionClickEvent;
  }
}

export default class DashboardMenuElement extends HTMLElement {
  constructor() {
    super();
  }

  connectedCallback() {
    const options = this.querySelectorAll("dashboard-option") as NodeListOf<DashboardOptionElement>;

    options.forEach((option) => {
      option.onclick = () => {
        if (option.target !== null) {
          this.classList.remove("open");
          this.dispatchEvent(new CustomEvent("option-click", {
            detail: { target: option.target },
            bubbles: true
          }));
        }
      };
    });

    this.addButton();
  }

  private addButton() {
    const button = document.createElement("button");
    button.className = "button";
    button.onclick = () => this.classList.toggle("open");

    const icon = new IconElement();
    icon.name = "arrow-up";

    button.append(icon);

    const options = this.querySelector("dashboard-options");

    if (options !== null) {
      options.prepend(button);
    }
  }
}

customElements.define("dashboard-menu", DashboardMenuElement);
