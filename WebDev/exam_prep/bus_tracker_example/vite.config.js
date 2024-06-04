import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react-swc'
import apiKey from './apiKey'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      '/api': {
        target: 'https://api.pugetsound.onebusaway.org/',
        secure: true,
        changeOrigin: true,
        rewrite: path => { 
          /* TODO: create a more sophisticated proxy that:
                1. properly inserts the key before the # in the URI
                2. does rate limiting per client
                3. only allows the paths we specifically want clients to use 
          */
          const separator = path.includes('?') ? '&' : '?'
          return `${path}${separator}key=${apiKey}`
        }
      }
    }
  }
})
