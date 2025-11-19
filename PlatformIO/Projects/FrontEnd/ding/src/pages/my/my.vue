<template>
	<view class="container">
		<button @click="backToHome">返回首页</button>

		<img ref="cameraImgRef" class="camera-img" />

		<view class="button-grid">
			<button class="btn btn-unlock" @click="handleOpenScreen('unlock')">解锁</button>
			<button class="btn btn-alt" @click="handleOpenScreen('2altTab')">2altTab</button>
			<button class="btn btn-app9" @click="handleOpenScreen('switchDk9')">切换到应用9</button>
			<button class="btn btn-app8" @click="handleOpenScreen('switchDk8')">切换到应用8</button>
			<button class="btn btn-app8" @click="handleOpenScreen('switchHome')">切换到桌面</button>
			<button class="btn btn-app8" @click="handleOpenScreen('swipeDing')">手动叮</button>
			<button class="btn btn-2esc" @click="handleOpenScreen('2esc')">两次取消</button>
			<button class="btn btn-esc" @click="handleOpenScreen('esc')">取消</button>
			<button class="btn btn-enter" @click="handleOpenScreen('enter')">确认</button>
			<button class="btn btn-clock" @click="handleOpenScreen('clockIn')">DK</button>
			<button class="btn btn-tab" @click="handleOpenScreen('tab')">tab</button>
			<button class="btn btn-menu" @click="handleOpenScreen('menu')">菜单</button>
			<button class="btn btn-back" @click="handleOpenScreen('minimizeWindow')">最小化</button>
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
const handleOpenScreen = async (oper: String) => {
	const body = {
		cmd: oper,
		deviceId: 'esp32-2805_A570_53B8'
	};
	await indexReq
		.postCommand(body)
		.then((res: any) => {
			console.log(res);
			// 没有拦截状态
			if (res.success == true) {
				// 消息提示
				uni.showToast({ title: '发送指令成功', icon: 'success' });
			} else {
				uni.showToast({ title: `发送指令失败${res.message}`, icon: 'fail' });
			}
		})
		.catch((err: any) => {
			console.log(err);
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
	width: 80%; /* 宽度占满屏幕 */
	height: auto; /* 高度自动，保持比例 */
	aspect-ratio: 4 / 3; /* 保持 4:3 比例 */
	display: block;
	margin: 10px auto;
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

/* 所有按钮统一尺寸 */
.btn {
	width: 100%;
	font-size: 22rpx;   /* 字体大小 */
	border-radius: 12rpx;
	color: #fff;
	text-align: center;
}

/* 每个按钮的独立风格 */
.btn-unlock { background: #4CAF50; }
.btn-alt { background: #2196F3; }
.btn-app9 { background: #9C27B0; }
.btn-app8 { background: #673AB7; }
.btn-2esc { background: #FF5722; }
.btn-esc { background: #FF9800; }
.btn-enter { background: #3F51B5; }
.btn-clock { background: #009688; }
.btn-tab { background: #795548; }
.btn-menu { background: #607D8B; }
.btn-back { background: #E91E63; }
</style>