import tailwindcss from '@tailwindcss/vite';
import { sveltekit } from '@sveltejs/kit/vite';
import { defineConfig } from 'vite';
import { readFileSync } from 'fs';

export default defineConfig({
    server: {
        host: true,  // allow lan access
        https: {
            key: readFileSync("../certs/dev-key.pem"),
            cert: readFileSync("../certs/dev-cert.pem"),
        }
    }, plugins: [tailwindcss(), sveltekit()]
});
