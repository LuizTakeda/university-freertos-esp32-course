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

