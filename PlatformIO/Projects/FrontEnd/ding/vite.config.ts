import { defineConfig } from "vite";
import uni from "@dcloudio/vite-plugin-uni";
import path from 'path';
// https://vitejs.dev/config/
export default defineConfig({
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