/** @type {import('tailwindcss').Config} */
export default {
  content: [
    './src/**/*.{html,js,svelte,ts}', // Make sure Tailwind scans your source files
    './public/index.html',  // Add paths to your HTML/Svelte files
  ],
  theme: {
    extend: {
      fontFamily: {
        sans: ['Roboto', 'sans-serif'],  // Add Roboto as the default sans font
      },
    },
  },
  plugins: [],
};
