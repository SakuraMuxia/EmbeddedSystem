import WebSocketManager from '@/utils/websocket';
// wsManager.ts —— 单独模块管理 WebSocket
let wsInstance:any = null;

export const getWebSocketInstance = () => {
	if (!wsInstance) {
		wsInstance = new WebSocketManager({
			url: `ws://39.106.41.164:3000/client`,
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