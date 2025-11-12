<template>
	<view class="container">
		<button @click="getConnections">获取连接列表</button>
		<scroll-view ref="scrollView" class="log-container" scroll-y scroll-top="0">
			<view v-for="(item, index) in logs" :key="index" class="log-item">
				<text class="log-text">{{ item.message }}</text>
				<text class="log-time">{{ item.time }}</text>
			</view>
		</scroll-view>
	</view>
</template>

<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount, nextTick } from 'vue';

const wsUrl = 'ws://39.106.41.164:3000/client';
const deviceId = 'browser01';
const heartbeatInterval = 15000;

const logs = ref<string[]>([]);
const scrollView = ref<any>(null);
const isConnected = ref(false);
let ws: ReturnType<typeof uni.connectSocket> | null = null;
let heartbeatTimer: number | null = null;
let reconnectTimer: number | null = null;


// 示例添加日志方法
function addLog(message) {
  const now = new Date();
  const timeStr = now.toLocaleTimeString(); // 时分秒
  logs.value.unshift({
    message,
    time: timeStr
  });

  // 滚动到顶部
  nextTick(() => {
    if (scrollView.value) scrollView.value.scrollTop = 0;
  });
}

/** ================= WebSocket ================= */
function initWebSocket() {
	if (ws && isConnected.value) {
		console.log('WebSocket 已连接，无需重复连接');
		return;
	}

	ws = uni.connectSocket({ url: wsUrl });
	console.log('WebSocket 对象已创建');

	uni.onSocketOpen(() => {
		console.log('WebSocket 已连接');
		isConnected.value = true;
		sendMessage('register', { deviceId });
		startHeartbeat();
	});

	uni.onSocketMessage((res) => {
		console.log('收到服务器消息:', res);
		// 处理 JSON 消息
		try {
			const msg = typeof res.data === 'string' ? JSON.parse(res.data) : res.data;
			if (msg.type === 'ping') {
				sendMessage('pong', { deviceId });
			} else {
				// 添加日志
				addLog(msg)
			}
		} catch (e) {
			logs.value.unshift(res.data);
		}
	});

	uni.onSocketClose(() => {
		console.warn('WebSocket 已关闭');
		isConnected.value = false;
		stopHeartbeat();
		ws = null;
		scheduleReconnect();
	});

	uni.onSocketError((err) => {
		console.error('WebSocket 错误:', err);
		isConnected.value = false;
		stopHeartbeat();
		ws = null;
		scheduleReconnect();
	});
}

function sendMessage(type: string, data: any) {
	if (!isConnected.value) return;
	const payload = {
		type,
		deviceId,
		...data
	};
	uni.sendSocketMessage({ data: JSON.stringify(payload) });
}

function startHeartbeat() {
	stopHeartbeat();
	heartbeatTimer = setInterval(() => {
		if (isConnected.value) {
			sendMessage('ping', {});
			console.log('发送心跳');
		}
	}, heartbeatInterval);
}

function stopHeartbeat() {
	if (heartbeatTimer) clearInterval(heartbeatTimer);
	heartbeatTimer = null;
}

function scheduleReconnect() {
	if (reconnectTimer) return;
	reconnectTimer = setTimeout(() => {
		reconnectTimer = null;
		console.log('尝试重新连接 WebSocket...');
		initWebSocket();
	}, 5000);
}
const getConnections = () => {
	sendMessage('listDevices', {});
};
/** ================= 页面生命周期 ================= */
onMounted(() => {
	initWebSocket();
});

onBeforeUnmount(() => {
	if (ws) uni.closeSocket();
	stopHeartbeat();
	if (reconnectTimer) clearTimeout(reconnectTimer);
});
</script>
<style scoped>
.container {
  flex: 1;
  padding: 10px;
}

.log-container {
  height: 100%;
}

.log-item {
  display: flex;
  flex-direction: row;
  justify-content: space-between;
  flex-wrap: wrap; /* 允许换行 */
  padding: 5px 0;
  border-bottom: 1px solid #eee;
}

.log-text {
  flex: 1 1 auto; /* 文本自适应宽度 */
  word-break: break-word; /* 换行显示 */
}

.log-time {
  flex: 0 0 auto;
  margin-left: 5px;
  color: #999;
  font-size: 12px;
}
</style>
