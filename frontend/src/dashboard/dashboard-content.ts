import DashboardPageElement from "./dashboard-page";

DashboardPageElement;

export default class DashboardContentElement extends HTMLElement {
  constructor() {
    super();
  }
}

customElements.define("dashboard-content", DashboardContentElement)