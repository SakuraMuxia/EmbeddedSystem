const express = require("express");
const cors = require("cors");
const WebSocket = require("ws");
const http = require("http");
const app = express();
app.use(express.json());



app.use(
  cors({
    origin: "*",
    methods: ["GET", "POST", "OPTIONS"],
    allowedHeaders: ["Content-Type", "Authorization"],
  })
);
app.options("*", (req, res) => res.sendStatus(200));

// ================= 配置 =================
const WS_PORT = 9001;
const HEARTBEAT_INTERVAL = 30000; // 30s 心跳
const MAX_FRAME_SIZE = 2 * 1024 * 1024; // 最大 2MB

const esp32Clients = new Set();
const h5Clients = new Set();

// 帧缓存，用于拼接分片
let espFrameBuffer = null;
let espFrameSize = 0;
let espReceived = 0;

// ================= WebSocket 帧处理 =================
function handleESP32FrameChunk(buf, h5Clients) {
  try {
    // 缓存未初始化，读取总长度
    if (!espFrameBuffer) {
      if (buf.length >= 4) {
        espFrameSize = buf.readUInt32BE(0);

        // 限制最大帧大小
        if (espFrameSize <= 0 || espFrameSize > MAX_FRAME_SIZE) {
          console.warn("[Node] Invalid frame size", espFrameSize);
          return false;
        }

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
        console.warn("[Node] Received unexpected data before length, ignoring", buf.length);
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
            try {
              client.send(espFrameBuffer, { binary: true });
            } catch (e) {
              console.error("[Node] send error:", e.message);
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
  } catch (err) {
    console.error("[Node] handleESP32FrameChunk exception:", err.message);
    // 重置缓存
    espFrameBuffer = null;
    espFrameSize = 0;
    espReceived = 0;
    return false;
  }
}

// ================= WebSocket 连接处理 =================
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
    if (clientType === "ESP32" && isBinary) {
      handleESP32FrameChunk(msg, h5Clients);
    } else if (clientType === "H5") {
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

// ================= 心跳机制 =================
setInterval(() => {
  const checkClients = (clients, type) => {
    clients.forEach((ws) => {
      if (!ws.isAlive) {
        console.log(`[WS] Terminating dead ${type} client`);
        ws.terminate();
        clients.delete(ws);
        return;
      }
      ws.isAlive = false;
      ws.ping(); // ESP32 和 H5 自动回应 pong
    });
  };

  checkClients(esp32Clients, "ESP32");
  checkClients(h5Clients, "H5");
}, HEARTBEAT_INTERVAL);

// ================= HTTP + WebSocket 服务 =================
const server = http.createServer(app); // 把 Express 绑定到 HTTP Server
const wss = new WebSocket.Server({ noServer: true });



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

app.post("api/stream/:action", (req, res) => {
  const action = req.params.action.toUpperCase();
  if (esp32Clients.size === 0) return res.status(500).json({ error: "No ESP32 connected" });

  esp32Clients.forEach((ws) => {
    if (ws.readyState === WebSocket.OPEN) ws.send(action);
  });

  res.json({ status: "OK", message: `Streaming ${action}` });
});

server.listen(WS_PORT, () => {
  console.log(`[HTTP+WS] Server listening on port ${WS_PORT}`);
});
