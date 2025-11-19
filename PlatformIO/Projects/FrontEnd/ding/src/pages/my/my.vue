<template>
	<view class="container">
		<button @click="backToHome">返回</button>
		<img ref="cameraImgRef" class="camera-img" />
		<view class="button-grid">
			<button @click="handleOpenScreen('unlock')">解锁</button>
			<button @click="handleOpenScreen('2altTab')">2altTab</button>
			<button @click="handleOpenScreen('switchDk9')">切换到应用9</button>
			<button @click="handleOpenScreen('switchDk8')">切换到应用8</button>
			<button @click="handleOpenScreen('2esc')">两次取消</button>
			<button @click="handleOpenScreen('esc')">取消</button>
			<button @click="handleOpenScreen('enter')">确认</button>
			<button @click="handleOpenScreen('clockIn')">DK</button>
			<button @click="handleOpenScreen('tab')">tab</button>
			<button @click="handleOpenScreen('menu')">菜单</button>
			<button @click="handleOpenScreen('minimizeWindow')">返回</button>
		</view>
	</view>
</template>

<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount, getCurrentInstance } from 'vue';
import WebSocketManager from '@/utils/websocket';
import indexReq from '@/api/index';
const cameraImgRef = ref<HTMLImageElement | null>(null);
let lastUrl: string | null = null;
let wsInstance: WebSocketManager | null = null;

const setupWebSocket = () => {
	console.log('开启视频websocket连接');
	wsInstance = new WebSocketManager({
		url: 'ws://39.106.41.164:9001/client',
		heartbeatInterval: 30000,
		reconnectInterval: 10000,
		maxReconnectAttempts: 5
	});
	wsInstance.binaryType = 'arraybuffer';
	wsInstance.onMessage((data) => {
		if (data instanceof ArrayBuffer) {
			const blob = new Blob([data], { type: 'image/jpeg' });
			const url = URL.createObjectURL(blob);
			if (cameraImgRef.value) cameraImgRef.value.src = url;
			if (lastUrl) URL.revokeObjectURL(lastUrl);
			lastUrl = url;
		}
	});
};
const handleOpenScreen = async (oper:String) => {
	
	const body = {
		cmd:oper,
		deviceId:"esp32-2805_A570_53B8"
	};
	await indexReq
		.postCommand(body)
		.then((res:any) => {
			console.log(res)
			// 没有拦截状态
			if(res.success == true){
				// 消息提示
				uni.showToast({ title: '发送指令成功', icon: 'success' });
			}else{
				uni.showToast({ title: `发送指令失败${res.message}`, icon: 'fail' });
			}
			
		})
		.catch((err:any) => {
			console.log(err)
			// 消息提示
			uni.showToast({ title: `网络错误`, icon: 'fail' });
		});
};
const closeWebSocket = () => {
	console.log('断开视频websocket连接');
	if (wsInstance) {
		wsInstance.close();
		wsInstance = null;
	}
	if (lastUrl) {
		URL.revokeObjectURL(lastUrl);
		lastUrl = null;
	}
};

onMounted(() => {
	// 初次进入页面也要打开连接
	setupWebSocket();
});

onBeforeUnmount(() => {
	closeWebSocket();
});

const backToHome = () => {
	uni.reLaunch({
		url: '/pages/index/index'
	});
};
</script>

<style scoped lang="scss">
	.camera-img {
	  width: 100%;       /* 宽度占满屏幕 */
	  height: auto;      /* 高度自动，保持比例 */
	  aspect-ratio: 4 / 3; /* 保持 4:3 比例 */
	  display: block;
	  margin: 10px 0;
	}
	.container {
	  padding: 10px;
	}
	
	.button-grid {
	  display: grid;
	  grid-template-columns: repeat(3, 1fr); /* 每行3列 */
	  gap: 10px; /* 行列间距 */
	  margin-top: 20px;
	}
	
	.button-grid button {
	  width: 100%;
	  padding: 10px;
	  font-size: 16px;
	}
	
</style>
