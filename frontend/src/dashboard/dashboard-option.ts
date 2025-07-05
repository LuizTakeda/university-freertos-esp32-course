export default class DashboardOptionElement extends HTMLElement {
  constructor() {
    super();
  }

  get target() {
    return this.getAttribute("target");
  }
}

customElements.define("dashboard-option", DashboardOptionElement);
