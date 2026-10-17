import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

export default defineConfig({
  plugins: [react()],
  base: '/app/',
  build: {
    outDir: '../../packages/platform/src/peykar/web_assets',
    emptyOutDir: true,
  },
  server: {
    proxy: {
      '/v1': 'http://127.0.0.1:8000',
      '/healthz': 'http://127.0.0.1:8000',
    },
  },
});
