export default class DashboardOptionElement extends HTMLElement {
  constructor() {
    super();
  }

  connectedCallback() {
  }

  get target() {
    return this.getAttribute("target");
  }
}

customElements.define("dashboard-option", DashboardOptionElement)