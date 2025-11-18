const net = require("net");
const WebSocket = require("ws");

const ESP_PORT = 9000; // ESP32 TCP 推流端口
const WS_PORT = 9001; // WebSocket 前端端口
const HEARTBEAT_INTERVAL = 30000; // 心跳检测间隔 30 秒

const esp32Clients = new Set();
const h5Clients = new Set();

const wss = new WebSocket.Server({ noServer: true });
console.log(`[WS] Server ready to handle /ESP32 and /client`);

// 帧缓存，用于拼接分片
let espFrameBuffer = null;
let espFrameSize = 0;
let espReceived = 0;

/**
 * 处理来自 ESP32 的 JPEG 分片数据
 * @param {Buffer} buf - Node 接收到的 Buffer
 * @param {Set<WebSocket>} h5Clients - 所有 H5 客户端集合
 */
function handleESP32FrameChunk(buf, h5Clients) {
  // 缓存未初始化，读取总长度
  if (!espFrameBuffer) {
    if (buf.length >= 4) {
      espFrameSize = buf.readUInt32BE(0);
      // console.log("espFrameSize", espFrameSize);
      // console.log(
      //   "buf.length",
      //   buf.length,
      //   "buf.slice(0,10)",
      //   buf.slice(0, 10)
      // );
      espFrameBuffer = Buffer.alloc(espFrameSize);
      espReceived = 0;

      // 有多余数据作为第一片
      if (buf.length > 4) {
        const firstChunk = buf.slice(4);
        firstChunk.copy(espFrameBuffer, 0);
        espReceived += firstChunk.length;
      }
      return false; // 尚未完整接收
    } else {
      console.warn(
        "[Node] Received unexpected data before length, ignoring",
        buf.length
      );
      return false;
    }
  }

  // 缓存已初始化，拼接分片
  buf.copy(espFrameBuffer, espReceived);
  espReceived += buf.length;

  // 收到完整 JPEG
  if (espReceived >= espFrameSize) {
    if (h5Clients.size > 0) {
      h5Clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
          // 这个就是实时视频流场景中，丢弃旧帧或覆盖帧比尝试缓冲所有帧更稳定、更节省内存
          try {
            client.send(espFrameBuffer, { binary: true }, (err) => {
              if (err) console.warn("send error", err);
            });
          } catch (e) {
            console.error(e);
          }
        }
      });
    }

    // 重置缓存
    espFrameBuffer = null;
    espFrameSize = 0;
    espReceived = 0;

    return true; // 完整帧已发送
  }

  return false; // 尚未完整接收
}

// 处理每个 WebSocket 连接
function handleConnection(ws, clientType) {
  ws.isAlive = true;

  if (clientType === "ESP32") esp32Clients.add(ws);
  else if (clientType === "H5") h5Clients.add(ws);

  console.log(
    `[WS] ${clientType} connected, total: ESP32=${esp32Clients.size}, H5=${h5Clients.size}`
  );

  ws.on("pong", () => {
    ws.isAlive = true;
    console.log(`[WS] Received PONG from ${clientType}`);
  });

  ws.on("message", (msg, isBinary) => {
    if (clientType === "ESP32") {
      // const buf = Buffer.from(msg);
      // console.log(buf)
      // handleESP32FrameChunk(msg, h5Clients);
      if (isBinary) {
        handleESP32FrameChunk(msg, h5Clients);
      }
    } else if (clientType === "H5") {
      // H5 客户端发送消息，可根据需求处理
      console.log("[H5] Message:", msg.toString());
    }
  });

  ws.on("close", () => {
    console.log(`[WS] ${clientType} disconnected`);
    if (clientType === "ESP32") esp32Clients.delete(ws);
    else if (clientType === "H5") h5Clients.delete(ws);
  });

  ws.on("error", (err) => {
    console.log(`[WS] ${clientType} error:`, err.message);
  });
}

// ================== 定时心跳检测 ==================
setInterval(() => {
  const checkClients = (clients, type) => {
    clients.forEach((ws) => {
      if (!ws.isAlive) {
        ws.missed += 1;
        if (ws.missed > 2) ws.terminate();
        console.log(`[WS] Terminating dead ${type} client`);
        ws.terminate();
        clients.delete(ws);
        return;
      }
      ws.isAlive = false;
      ws.missed = 0;
      ws.ping(); // ESP32 和 H5 自动回应 pong
    });
  };

  checkClients(esp32Clients, "ESP32");
  checkClients(h5Clients, "H5");
}, HEARTBEAT_INTERVAL);

// ================== HTTP 升级处理 ==================
const http = require("http");
const server = http.createServer();

server.on("upgrade", (request, socket, head) => {
  const url = request.url;

  if (url === "/ESP32") {
    wss.handleUpgrade(request, socket, head, (ws) => {
      handleConnection(ws, "ESP32");
    });
  } else if (url === "/client") {
    wss.handleUpgrade(request, socket, head, (ws) => {
      handleConnection(ws, "H5");
    });
  } else {
    socket.destroy();
  }
});

server.listen(WS_PORT, () => {
  console.log(`[HTTP] Server listening on port ${WS_PORT}`);
});
