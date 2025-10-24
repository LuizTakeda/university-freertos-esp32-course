import "./dashboard.scss";
import DashboardContentElement from "./dashboard-content";
import DashboardMenuElement, { OptionClickEvent } from "./dashboard-menu";

DashboardMenuElement;
DashboardContentElement;

export default class DashboardStruct extends HTMLElement {
  private menu: DashboardMenuElement | null = null;
  private content: DashboardContentElement | null = null;

  constructor() {
    super();
  }

  connectedCallback() {
    this.getMenu();
    this.getContent();

    this.menu!.addEventListener("option-click", (event) => {
      this.content!.setActive(event.detail.target);
    })
  }

  private getMenu() {
    this.menu = this.querySelector("dashboard-menu") as DashboardMenuElement | null;
    if (this.menu === null) {
      throw new Error("Dashboard menu is null");
    }
  }

  private getContent() {
    this.content = this.querySelector("dashboard-content") as DashboardContentElement | null;
    if (this.content === null) {
      throw new Error("Dashboard content is null");
    }
  }
}

customElements.define("dashboard-struct", DashboardStruct);
