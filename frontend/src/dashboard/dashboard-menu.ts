import DashboardOptionElement from "./dashboard-option";

DashboardOptionElement;

type OptionClickCallback = (name: string) => void;

export default class DashboardMenuElement extends HTMLElement {
  private onOptionClickCallback: OptionClickCallback = (_) => { };

  constructor() {
    super();
  }

  connectedCallback() {
    const options = this.querySelectorAll("dashboard-option") as NodeListOf<DashboardOptionElement>;

    options.forEach((option) => {
      option.onclick = () => {
        console.log("Botao", option.target);
      }
    });
  }

  onOptionClick(callback: OptionClickCallback) {
    this.onOptionClickCallback = callback;
  }
}

customElements.define("dashboard-menu", DashboardMenuElement)