# Web Dashboard

This dashboard is built using [Web Components](https://developer.mozilla.org/en-US/docs/Web/API/Web_components) for modular UI elements, [SASS](https://sass-lang.com/) for styling, and [Webpack](https://webpack.js.org/) to bundle and minify the project into a single JavaScript file.

This setup ensures a minimal file size, making it ideal for embedded platforms like the ESP32.

## Development

### Install dependencies

```bash
$ pnpm install
```

### Start the development server

Runs both the Webpack Dev Server and an Express app that emulates the ESP32 REST API:

```bash
$ pnpm run dev
```

## Build

To generate the final `index.html` and `bundle.js` files for deployment:

```bash
$ pnpm run build
```

The build generates:
- `index.html`: Entry HTML page
- `bundle.js`: Minified JavaScript file containing all components and logic

## Web Components Overview

The dashboard UI is composed of custom Web Components.

### Dashboard Element

| Tag                  | Description                                     |
|----------------------|-------------------------------------------------|
| `<dashboard-struct>` | Root container that wires up the layout         |
| `<dashboard-menu>`   | Holds navigation options                        |
| `<dashboard-option>` | Selects a page by `target` attribute            |
| `<dashboard-content>`| Contains the pages, shows one at a time         |
| `<dashboard-page>`   | A page identified by the `name` attribute       |

### Example Usage

```html
<dashboard-struct>
  <!-- Menu -->
  <dashboard-menu>
    <dashboard-option target="home">Home</dashboard-option>
    <dashboard-option target="i/o">Inputs</dashboard-option>
  </dashboard-menu>

  <!-- Content -->
  <dashboard-content default="home">
    <!-- Home Page -->
    <dashboard-page name="home">
      <h1>Home Page</h1>
    </dashboard-page>

    <!-- I/O Page -->
    <dashboard-page name="i/o">
      <h1>I/O Page</h1>
    </dashboard-page>
  </dashboard-content>
</dashboard-struct>
```

## Gallery

### Desktop
<!-- <img width="1920" height="1080" src="https://github.com/user-attachments/assets/7146b382-f8f5-460a-bc46-5b8e12310341" /> -->
<!-- <img width="1920" height="1080" src="https://github.com/user-attachments/assets/8975d8c6-c57c-44df-a68a-540566f63f01" /> -->


### Mobile  
<!-- <div>
  <img width="300" src="https://github.com/user-attachments/assets/b91b1cf1-b31a-43e2-832a-200d8805cee4" />
  <img width="300" src="https://github.com/user-attachments/assets/4f1af473-ec35-4179-91e2-b9761fa9599d" />
  
</div> -->

<!-- <div>
  <img width="300" src="https://github.com/user-attachments/assets/bac0e8e7-3791-44de-aca5-db9429861d0a" />  
  <img width="300" src="https://github.com/user-attachments/assets/e6ac00b0-6175-4172-84e5-d7867c0dec91" />
</div> -->
