<template>
	<view class="container">
		<view class="header">
			<text class="title">ESP32 实时日志</text>
			<button class="btn" @click="connectWS">连接</button>
		</view>

		<scroll-view class="log-box" scroll-y="true" :scroll-top="scrollTop">
			<view v-for="(item, index) in logs" :key="index" class="log-item">
				<text>[{{ item.time }}] {{ item.msg }}</text>
			</view>
		</scroll-view>
	</view>
</template>

<script setup>
import { ref } from 'vue';

const logs = ref([]);
const scrollTop = ref(0);
let ws = null;

// ✅ WebSocket 地址（换成你的 Node.js 服务器 IP）
const WS_URL = 'ws://192.168.1.245:3000';

function appendLog(msg) {
	logs.value.push({
		time: new Date().toLocaleTimeString(),
		msg
	});
	scrollTop.value = logs.value.length * 100;
}

// ✅ 建立 WebSocket 连接
function connectWS() {
	if (ws) {
		ws.close();
		ws = null;
	}

	ws = uni.connectSocket({
		url: WS_URL,
		success() {
			appendLog('✅ 尝试连接 WebSocket...');
		}
	});

	// 监听事件
	ws.onOpen(() => {
		appendLog('🔗 WebSocket 已连接');
	});

	ws.onMessage((res) => {
		try {
			const data = JSON.parse(res.data);
			appendLog('📩 收到日志: ' + JSON.stringify(data));
		} catch (e) {
			appendLog('📩 收到原始信息: ' + res.data);
		}
	});

	ws.onClose(() => {
		appendLog('⚠️ WebSocket 已断开');
	});

	ws.onError((err) => {
		appendLog('❌ WebSocket 错误: ' + JSON.stringify(err));
	});
}
</script>

<style lang="scss" scoped>
.container {
  display: flex;
  flex-direction: column;
  height: 100vh;
  background-color: #111;
  color: #0f0;
  padding: 20rpx;
}
.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
.title {
  font-size: 36rpx;
  color: #0f0;
}
.btn {
  background-color: #222;
  color: #0f0;
  padding: 12rpx 24rpx;
  border-radius: 12rpx;
}
.log-box {
  flex: 1;
  margin-top: 20rpx;
  border: 1rpx solid #333;
  background-color: #000;
  border-radius: 12rpx;
  padding: 16rpx;
}
.log-item {
  margin-bottom: 12rpx;
}	

</style>
