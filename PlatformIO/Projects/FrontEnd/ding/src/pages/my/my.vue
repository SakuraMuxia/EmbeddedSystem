<template>
	<view class="container">
	    <img ref="cameraImgRef" width="320" height="240" />
	  </view>
</template>

<script setup>
import {ref, onMounted, onBeforeUnmount } from 'vue';
import WebSocketManager from '@/utils/websocket';
var wsInstance;
// Canvas 宽高
const imgSrc = ref('')
const cameraImgRef = ref()
const getWebSocketInstance = () => {
	if (true) {
		wsInstance = new WebSocketManager({
			url: `ws://192.168.2.250:9001/client`,
			heartbeatInterval: 30000,
			reconnectInterval: 10000,
			maxReconnectAttempts: 5
		});

		console.warn('[WS] WebSocket 已初始化');
	} else {
		console.warn('[WS] 已存在连接，复用');
	}
	return wsInstance;
};
let lastUrl = null;
const ws = getWebSocketInstance();
ws.binaryType = 'arraybuffer';
ws.onMessage((data) => {
	
	if (data instanceof ArrayBuffer) {
		console.log('data',data)
		const blob = new Blob([data], { type: 'image/jpeg' });
		const url = URL.createObjectURL(blob)
		if (cameraImgRef.value) cameraImgRef.value.src = url;
		if (lastUrl) URL.revokeObjectURL(lastUrl);
		lastUrl = url;
	}
});

onMounted(() => {
	
});
onBeforeUnmount(() => {
	if (ws && ws.readyState === WebSocket.OPEN) {
		ws.close();
	}
});
</script>

<style lang="scss" scoped></style>
