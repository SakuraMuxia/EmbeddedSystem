import { defineConfig } from "vite";
import uni from "@dcloudio/vite-plugin-uni";
import path from 'path';
// https://vitejs.dev/config/
export default defineConfig({
	server: {
		proxy: {
			'/api': {
				target: 'http://39.106.41.164:3000',
				changeOrigin: true,
				rewrite: (path) => path.replace(/^\/api/, '/api')
			}
		}
	},
	plugins: [uni()],
	css: {
		preprocessorOptions: {
			scss: {
				// 忽略 deprecation 警告
				quietDeps: true
			}
		}
	}
});