import DashboardPageElement from "./dashboard-page";

DashboardPageElement;

export default class DashboardContentElement extends HTMLElement {
  private pages: NodeListOf<DashboardPageElement> | null = null;

  constructor() {
    super();
  }

  connectedCallback() {
    this.pages = this.querySelectorAll("dashboard-page") as NodeListOf<DashboardPageElement>;

    this.pages.forEach((page) => {
      page.setActive(page.name === this.default);
    });
  }

  setActive(name: string) {
    this.pages?.forEach((page) => {
      page.setActive(page.name === name);
    });
  }

  get default() {
    return this.getAttribute("default");
  }
}

customElements.define("dashboard-content", DashboardContentElement);
