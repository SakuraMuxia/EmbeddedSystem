// server.js
const express = require("express");
const http = require("http");
const WebSocket = require("ws");
const cors = require("cors");

const app = express();
app.use(cors());
app.use(express.json());

const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

const PORT = process.env.PORT || 3000;

// 存储连接
const espMap = new Map();    // deviceId -> ws (ESP 连接)
const clientSet = new Set(); // 浏览器客户端集合 (接收日志)

// 心跳设置 (用于清理断开的 ws)
const HEARTBEAT_INTERVAL = 30000; // 30s

// --- WebSocket connection handling ---
// We use URL path to differentiate ESP 和 浏览器 client:
// ws://host:port/esp  -> ESP32 devices
// ws://host:port/client -> browser log clients


wss.on("connection", function connection(ws, req) {
  const url = req.url || "";

  // mark alive for heartbeat
  ws.isAlive = true;
  ws.on("pong", () => {
    ws.isAlive = true;
  });

  if (url === "/esp") {
    console.log("🔌 New ESP connection (unregistered)");
    // Temporarily store unregistered ESP under a Symbol key to find its ws on close
    ws._registeredId = null;

    ws.on("message", (raw) => {
      let msg;
      try {
        msg = JSON.parse(raw.toString());
      } catch (e) {
        console.warn("Invalid JSON from ESP:", raw.toString());
        return;
      }

      // Register message: { type: "register", deviceId: "esp01" }
      if (msg.type === "register" && msg.deviceId) {
        ws._registeredId = msg.deviceId;
        espMap.set(msg.deviceId, ws);
        console.log(`✅ Registered device: ${msg.deviceId}`);
        // Broadcast device-online message to browser clients
        broadcastToClients(JSON.stringify({
          type: "device",
          event: "online",
          deviceId: msg.deviceId
        }));
        return;
      }

      // Log/result message from ESP: { type: "log"|"result", deviceId:"esp01", data:"..." }
      if (msg.type === "log" || msg.type === "result" || msg.type === "status") {
        // Add server timestamp for convenience
        const out = {
          ...msg,
          ts: Date.now()
        };
        // Broadcast to all browser clients
        broadcastToClients(JSON.stringify(out));
        console.log("Esp32回传数据:",JSON.stringify(out))
        // Optionally: you might want to persist logs here (DB / file)
        return;
      }

      // If receives cmd ack etc, just broadcast
      broadcastToClients(JSON.stringify({ from: msg.deviceId || ws._registeredId || "esp", raw: msg }));
      console.log("Esp32回传数据:",JSON.stringify({ from: msg.deviceId || ws._registeredId || "esp", raw: msg }))
    });

    ws.on("close", () => {
      if (ws._registeredId) {
        console.log(`❌ ESP disconnected: ${ws._registeredId}`);
        espMap.delete(ws._registeredId);
        broadcastToClients(JSON.stringify({
          type: "device",
          event: "offline",
          deviceId: ws._registeredId,
          ts: Date.now()
        }));
      } else {
        console.log("❌ An unregistered ESP disconnected");
      }
    });

    ws.on("error", (err) => {
      console.warn("ESP socket error:", err.message || err);
    });
  }

  else if (url === "/client") {
    console.log("🌐 New browser client connected");
    clientSet.add(ws);

    ws.on("message", (raw) => {
      // 可选：允许浏览器通过 websocket 订阅、取消订阅或发起一些交互
      // 例如 { type: "ping" } 或 { type:"listDevices" }
      try {
        const msg = JSON.parse(raw.toString());
        if (msg && msg.type === "listDevices") {
          // 返回当前在线设备列表
          const devices = [...espMap.keys()];
          ws.send(JSON.stringify({ type: "deviceList", devices }));
        }
      } catch (e) {
        // ignore non-json or arbitrary messages
      }
    });

    ws.on("close", () => {
      clientSet.delete(ws);
      console.log("🌐 Browser client disconnected");
    });

    ws.on("error", (err) => {
      clientSet.delete(ws);
      console.warn("Browser client ws error:", err.message || err);
    });
  }

  else {
    // 未识别路径的 ws 直接关闭
    console.log("Unknown ws path:", url);
    ws.close();
  }
});
// 广播给所有浏览器客户端（/client）
function broadcastToClients(message) {
  for (const c of clientSet) {
    if (c.readyState === WebSocket.OPEN) {
      try {
        c.send(message);
      } catch (e) {
        console.warn("Failed to send to client:", e.message || e);
      }
    }
  }
}

// --- HTTP API ---
// POST /api/sendCmd
// body: { deviceId: "esp01", cmd: "move_up", meta: {...} }
// 如果设备在线则立即通过 ws 转发：{ type: "cmd", data: "...", meta: {...} }
app.post("/api/operphone", (req, res) => {
  const { deviceId, cmd, meta } = req.body || {};
  if (!deviceId || !cmd) {
    res.status(400).json({ success: false, message: "需要 deviceId 和 cmd" });
    return;
  }

  const espWs = espMap.get(deviceId);
  if (!espWs || espWs.readyState !== WebSocket.OPEN) {
    res.status(404).json({ success: false, message: `设备 ${deviceId} 不在线` });
    return;
  }

  const payload = JSON.stringify({
    type: "cmd",
    deviceId,
    action: cmd,
    meta: meta || {},
    ts: Date.now()
  });

  try {
    espWs.send(payload);
    // 同时把发送事件写入到浏览器日志流（通知所有浏览器）
    broadcastToClients(JSON.stringify({
      type: "server",
      event: "sentCmd",
      deviceId,
      action: cmd,
      ts: Date.now()
    }));

    res.json({ success: true, message: "命令已发送" });
  } catch (e) {
    console.error("Failed to send cmd to esp:", e);
    res.status(500).json({ success: false, message: "发送命令失败" });
  }
});

// 查询在线设备
app.get("/api/devices", (req, res) => {
  res.json({ devices: [...espMap.keys()] });
});

// health
app.get("/api/health", (req, res) => res.json({ ok: true, ts: Date.now() }));

// --- Heartbeat clean-up for dead sockets ---
// Regularly ping clients and esp sockets; if no pong within interval mark isAlive false and terminate.
const interval = setInterval(() => {
  wss.clients.forEach((socket) => {
    if (socket.isAlive === false) {
      console.log("Terminating dead socket");
      return socket.terminate();
    }

    socket.isAlive = false;
    try {
      socket.ping();
    } catch (e) {
      // may throw if socket already closing
    }
  });
}, HEARTBEAT_INTERVAL);

// Cleanup on exit
process.on("SIGINT", () => {
  clearInterval(interval);
  console.log("Shutting down server...");
  wss.close();
  server.close(() => process.exit(0));
});

server.listen(PORT, () => {
  console.log(`🚀 Server listening on http://0.0.0.0:${PORT}`);
  console.log(`WebSocket paths: ws://<host>:${PORT}/esp  and  ws://<host>:${PORT}/client`);
});
