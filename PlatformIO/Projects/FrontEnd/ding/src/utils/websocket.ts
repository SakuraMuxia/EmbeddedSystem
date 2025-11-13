interface WSOptions {
  url: string;
  heartbeatInterval?: number;   // 心跳间隔(ms)
  reconnectInterval?: number;   // 重连间隔(ms)
  maxReconnectAttempts?: number; // 最大重连次数
}
type MessageCallback = (data: any) => void;

class WebSocketManager {
  private url: string;
  private heartbeatInterval: number;
  private reconnectInterval: number;
  private maxReconnectAttempts: number;

  private socketOpen = false;
  private heartbeatTimer: number | null = null;
  private reconnectTimer: number | null = null;
  private reconnectAttempts = 0;
  private isManuallyClosed = false; // 手动关闭标记
  private messageCallback: MessageCallback | null = null;

  constructor(options: WSOptions) {
    this.url = options.url;
    this.heartbeatInterval = options.heartbeatInterval || 15000;
    this.reconnectInterval = options.reconnectInterval || 5000;
    this.maxReconnectAttempts = options.maxReconnectAttempts || 5;

    this.initSocket();
  }

  private initSocket() {
	  if (this.socketOpen || this.reconnectTimer) {
		  console.warn('[WS] WebSocket 已存在或正在重连，跳过 initSocket');
		  return;
		}
    if (!this.url) {
      console.error('[WS] URL 未设置');
      return;
    }
    if (this.isManuallyClosed) {
      console.warn('[WS] 已手动关闭，不再连接');
      return;
    }

    // 创建 WebSocket
    uni.connectSocket({
      url: this.url,
      success: () => console.log('[WS] 正在连接...'),
      fail: (err) => {
        console.error('[WS] 初始连接失败', err);
        this.scheduleReconnect();
      },
    });

    uni.onSocketOpen(() => {
      console.log('[WS] 连接已打开');
      this.socketOpen = true;
      this.reconnectAttempts = 0;
      this.startHeartbeat();
    });

    uni.onSocketMessage((res) => {
      let data: any;
      try {
        data = typeof res.data === 'string' ? JSON.parse(res.data) : res.data;
      } catch (e) {
        console.warn('[WS] 非 JSON 消息', res.data);
        return;
      }
    
      if (data.type === 'ping') {
        this.send({ type: 'pong', ts: Date.now() });
        console.log('[WS] 收到服务器 ping → 已回复 pong');
        return;
      }
    
      if (this.messageCallback) this.messageCallback(data);
    });

    uni.onSocketError((err) => {
      console.error('[WS] 连接错误', err);
      this.socketOpen = false;
      this.scheduleReconnect();
    });

    uni.onSocketClose(() => {
      console.warn('[WS] 连接关闭');
      this.socketOpen = false;
      this.stopHeartbeat();
      this.scheduleReconnect();
    });
  }

  /** 发送消息 */
  public send(data: any) {
    if (!this.socketOpen) {
      console.warn('[WS] 连接未打开，发送失败');
      return;
    }
    const payload = typeof data === 'string' ? data : JSON.stringify(data);
    uni.sendSocketMessage({
      data: payload,
      success: () => console.log('[WS] 消息发送成功', payload),
      fail: (err) => console.error('[WS] 消息发送失败', err),
    });
  }

  /** 注册消息回调 */
  public onMessage(callback: MessageCallback) {
    this.messageCallback = callback;
  }

  /** 启动心跳 */
  private startHeartbeat() {
    this.stopHeartbeat();

    this.heartbeatTimer = setInterval(() => {
      if (this.socketOpen) {
        this.send({ type: 'heartbeat'});
        console.log('[WS] 心跳发送');
      }
    }, this.heartbeatInterval);
  }

  /** 停止心跳 */
  private stopHeartbeat() {
    if (this.heartbeatTimer) {
      clearInterval(this.heartbeatTimer);
      this.heartbeatTimer = null;
    }
  }

  /** 重连逻辑 */
  private scheduleReconnect() {
	if (this.socketOpen) return; // 已连接就不重连
    if (this.isManuallyClosed) return;
    if (this.reconnectAttempts >= this.maxReconnectAttempts) {
      console.error('[WS] 重连次数过多，停止重连');
      return;
    }
    if (this.reconnectTimer) return; // 避免重复定时器

    this.reconnectAttempts++;
    console.warn(`[WS] 尝试第 ${this.reconnectAttempts} 次重连...`);
    this.reconnectTimer = setTimeout(() => {
      this.reconnectTimer = null;
      this.initSocket();
    }, this.reconnectInterval);
  }

  /** 手动关闭 WebSocket */
  public close() {
    this.isManuallyClosed = true;
    this.stopHeartbeat();
    if (this.reconnectTimer) clearTimeout(this.reconnectTimer);
    uni.closeSocket();
    this.socketOpen = false;
    console.log('[WS] 手动关闭连接');
  }
}

export default WebSocketManager;