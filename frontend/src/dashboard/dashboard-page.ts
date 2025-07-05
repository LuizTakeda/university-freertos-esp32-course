export default class DashboardPageElement extends HTMLElement {
  constructor() {
    super();

    this.setActive(false);
  }

  get name() {
    return this.getAttribute("name");
  }

  setActive(value: boolean) {
    this.style.display = value ? "block" : "none";
  }
}

customElements.define("dashboard-page", DashboardPageElement);
