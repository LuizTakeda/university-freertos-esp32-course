import "./theme-changer.scss"

// Type for valid theme names
export type Theme = "dark-theme" | "light-theme";

// Custom event type for theme changes
type ThemeChangeEvent = CustomEvent<{ theme: Theme }>;

// Extend global event map to include 'theme-change'
declare global {
  interface WindowEventMap {
    "theme-change": ThemeChangeEvent;
  }
}

export default class ThemeChangerElement extends HTMLElement {
  private static theme: Theme;

  constructor() {
    super();
  }

  connectedCallback() {
    // Set up click handler and event listener
    this.onclick = () => this.changeTheme();
    window.addEventListener("theme-change", this.handleThemeChange);

    // Load theme on component attach
    ThemeChangerElement.loadTheme();
  }

  disconnectedCallback() {
    // Clean up listener when element is removed
    window.removeEventListener("theme-change", this.handleThemeChange);
  }

  private static loadTheme() {
    let themeStoraged = localStorage.getItem("theme") as Theme | null;

    // Use stored theme or system preference
    if (themeStoraged === null) {
      const dark = window.matchMedia("(prefers-color-scheme: dark)").matches;
      this.theme = dark ? "dark-theme" : "light-theme";
      localStorage.setItem("theme", this.theme);
    } else {
      this.theme = themeStoraged;
    }

    // Apply theme class and notify listeners
    document.documentElement.classList.add(this.theme);
    window.dispatchEvent(new CustomEvent("theme-change", {
      detail: { theme: this.theme }
    }));
  }

  private changeTheme() {
    // Toggle between light and dark themes
    document.documentElement.classList.remove("dark-theme", "light-theme");
    ThemeChangerElement.theme = ThemeChangerElement.theme === "dark-theme" ? "light-theme" : "dark-theme";
    document.documentElement.classList.add(ThemeChangerElement.theme);
    localStorage.setItem("theme", ThemeChangerElement.theme);

    // Dispatch theme change event
    window.dispatchEvent(new CustomEvent("theme-change", {
      detail: { theme: ThemeChangerElement.theme }
    }));
  }

  // Update element text when theme changes
  private handleThemeChange = (event: ThemeChangeEvent) => {
    this.innerText = event.detail.theme;
  };
}

customElements.define("theme-changer", ThemeChangerElement);
