import { defineConfig } from "vite";
import uni from "@dcloudio/vite-plugin-uni";
import path from 'path';
// https://vitejs.dev/config/
export default defineConfig({
	plugins: [uni()],
	resolve: {
		alias: {
			'@': path.resolve(__dirname, 'src'),
			'@dcloudio/uni-ui': path.resolve(__dirname, 'node_modules/@dcloudio/uni-ui')
		}
	},
	optimizeDeps: {
		include: ['@dcloudio/uni-ui']
	}
});