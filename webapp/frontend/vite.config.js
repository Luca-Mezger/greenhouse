import { sveltekit } from '@sveltejs/kit/vite';
import { defineConfig } from 'vitest/config';

export default defineConfig({
	plugins: [sveltekit()],
	server: {
		host: '0.0.0.0', // This will allow access from other devices on the network
		port: 3000        // Ensure this is the port you want to use
	},
	test: {
		include: ['src/**/*.{test,spec}.{js,ts}']
	}
});
