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
          this.dispatchEvent(new CustomEvent("option-click", {
            detail: { target: option.target },
            bubbles: true
          }));
        }
      };
    });
  }
}

customElements.define("dashboard-menu", DashboardMenuElement);
