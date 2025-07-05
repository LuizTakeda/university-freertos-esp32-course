
class MyElement extends HTMLElement {
  constructor() {
    super();

    this.attachShadow({ mode: "open" });

    if (this.shadowRoot) {
      this.shadowRoot.innerHTML = `
      <style>
        h1 { color: crimson; }
      </style>
      <h1>Hello from MyElement HAHAHA!</h1>
    `;

      fetch("/api", {
        method: "GET",
      })
        .then(async (response) => {
          if (this.shadowRoot)
            this.shadowRoot.innerHTML += `<h2>Success GET API (${await response.text()})</h2>`
        })
        .catch((error) => {
          if (this.shadowRoot)
            this.shadowRoot.innerHTML += `<h2>Failled GET API</h2>`
        });
    }

  }
}

customElements.define('my-element', MyElement);