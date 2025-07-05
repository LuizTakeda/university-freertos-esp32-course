import DashboardContentElement from "./dashboard-content";
import DashboardMenuElement from "./dashboard-menu";

DashboardContentElement;
DashboardMenuElement;

export default class DashboardStruct extends HTMLElement {
  constructor() {
    super();
  }

  connectedCallback() {
    this.style.display = "block";
    this.style.border = "solid 1px red"

    const menu = this.querySelector("dashboard-menu") as DashboardMenuElement | null;
    if (menu === null) {
      throw new Error("Dashboard menu is null");
    }

    const content = this.querySelector("dashboard-content") as DashboardContentElement | null;
    if (content === null) {
      throw new Error("Dashboard content is null");
    }

    menu.onOptionClick((optionName) => {
      console.log(optionName);
    })
  }
}

customElements.define("dashboard-struct", DashboardStruct)