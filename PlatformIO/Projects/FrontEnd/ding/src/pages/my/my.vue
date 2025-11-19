<template>
  <view class="container">
	  <button @click="backToHome">返回</button>
    <img ref="cameraImgRef" width="400" height="320" />
  </view>
</template>

<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount, getCurrentInstance } from 'vue';
import WebSocketManager from '@/utils/websocket';

const cameraImgRef = ref<HTMLImageElement | null>(null);
let lastUrl: string | null = null;
let wsInstance: WebSocketManager | null = null;

const setupWebSocket = () => {
  console.log("开启视频websocket连接")
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

const closeWebSocket = () => {
	console.log("断开视频websocket连接")
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
		url:"/pages/index/index"
	})
}
</script>

<style scoped lang="scss"></style>
