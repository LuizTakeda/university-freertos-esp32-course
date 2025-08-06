export default class FormElement extends HTMLFormElement {
  constructor() {
    super();
  }

  connectedCallback() {
    this.fetch();
  }

  async fetch() {
    const path = this.getAttribute("path");

    if (path === null) {
      console.error(`${path}: path not found`);
      return;
    }

    const response = await fetch(path, { method: "GET" });

    if (!response.ok) {
      console.error(`${path}: error on fetch`);
      return;
    }

    let data;

    try {
      data = await response.json();
    } catch (_) {
      console.error(`${path}: invalid json`);
      return;
    }

    console.log(data);
  }
}

customElements.define("form-element", FormElement, { extends: "form" });