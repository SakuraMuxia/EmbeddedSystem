<template>
	<view class="container">
		
		
	</view>
</template>

<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount, nextTick } from 'vue';
import req from '@/api/index'
import {getWebSocketInstance} from '@/utils/getSocketIns'

const ws = getWebSocketInstance()

// 注册函数
const registClient = () => {
  if (ws.socketOpen) {
    ws.send({ type: 'register', clientId: 'browser_001' });
  } else {
    setTimeout(registClient, 500);
  }
};

// 接收消息
ws.onMessage((data) => {
  let msg;
  try {
    msg = typeof data === 'string' ? JSON.parse(data) : data;
  } catch (e) {
    console.warn('收到非 JSON 消息', data);
    return;
  }
  console.log("msg",msg)
});

// 发送消息
function sendMessage(action: string, payload: any) {
  if (!ws.socketOpen) {
    console.warn('WebSocket 未连接，消息发送失败', action);
    return;
  }
  ws.send({ type: action, ...payload, ts: Date.now() });
}


onMounted(()=>{
	registClient();
})

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
