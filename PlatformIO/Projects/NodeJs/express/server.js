const express = require("express");
const http = require("http");
const WebSocket = require("ws");
const cors = require("cors");

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

const server = http.createServer(app);
const wss = new WebSocket.Server({ server });
const PORT = process.env.PORT || 3000;

const espMap = new Map(); // deviceId -> ws
const clientMap = new Map(); // clientId -> ws

// 心跳参数
const HEARTBEAT_INTERVAL = 3000; // 30s
const PONG_TIMEOUT = 6000; // 60s

// 简单 token 生成（真实情况你可以改成 JWT）
function generateToken() {
  return Math.random().toString(36).substring(2) + Date.now().toString(36);
}

// 内存 token 存储：token -> clientId
const tokenMap = new Map();

// 广播消息给所有浏览器客户端
function broadcastToClients(message) {
  for (const ws of clientMap.values()) {
    if (ws.readyState === WebSocket.OPEN) {
      try {
        ws.send(message);
      } catch (e) {
        console.warn("Failed to send:", e.message || e);
      }
    }
  }
}
// 封装响应数据类型
function buildDeviceMessage(
  type,
  deviceId,
  action,
  result = "success",
  msg = ""
) {
  return JSON.stringify({
    type,
    deviceId,
    ts: Date.now(),
    action, // 执行动作
    result, // success / fail
    msg, // 扩展字段
  });
}
// 格式化时间戳
function formatTimestamp(ts) {
  const date = new Date(ts);
  const Y = date.getFullYear();
  const M = String(date.getMonth() + 1).padStart(2, "0");
  const D = String(date.getDate()).padStart(2, "0");
  const h = String(date.getHours()).padStart(2, "0");
  const m = String(date.getMinutes()).padStart(2, "0");
  const s = String(date.getSeconds()).padStart(2, "0");
  return `${Y}-${M}-${D} ${h}:${m}:${s}`;
}
// WebSocket 连接处理
wss.on("connection", (ws, req) => {
  ws.isAlive = true;
  ws.lastPong = Date.now();

  ws.on("pong", () => {
    ws.isAlive = true;
    ws.lastPong = Date.now();
    const formattedTime = formatTimestamp(ws.lastPong);
    console.log(
      `${formattedTime}✅ 收到 ${
        ws._registeredId || ws._clientId || "未注册"
      } 的 pong`
    );
  });

  const url = req.url || "";

  if (url === "/esp") {
    ws._registeredId = null;

    ws.on("message", (raw) => {
      let msg;
      try {
        msg = JSON.parse(raw.toString());
      } catch (e) {
        console.warn("Invalid JSON from ESP:", raw.toString());
        return;
      }
      // 当收到ESP32的注册消息时
      if (msg.type === "register" && msg.deviceId) {
        ws._registeredId = msg.deviceId;

        // 断开旧连接
        if (espMap.has(msg.deviceId)) {
          const oldWs = espMap.get(msg.deviceId);

          // 如果旧连接仍然是OPEN状态，并且不是当前连接
          if (oldWs !== ws && oldWs.readyState === WebSocket.OPEN) {
            console.log(`⚠️ 发现重复 deviceId，断开旧连接: ${msg.deviceId}`);
            oldWs.terminate();
          }
        }

        espMap.set(msg.deviceId, ws);
        console.log(`✅ ESP32上线: ${msg.deviceId}`);

        broadcastToClients(
          buildDeviceMessage("device", msg.deviceId, "online")
        );
        return;
      }
      // 当收到ESP32的日志消息时
      if (
        msg.type === "log" ||
        msg.type === "result" ||
        msg.type === "status"
      ) {
        broadcastToClients(
          buildDeviceMessage(
            "device",
            msg.deviceId,
            msg.action || "",
            msg.result || "success",
            msg.message || ""
          )
        );
      }
    });

    ws.on("close", () => {
      if (ws._registeredId) {
        espMap.delete(ws._registeredId);
        broadcastToClients(
          buildDeviceMessage("device", ws._registeredId, "offline")
        );
        console.log(`ESP32断开: ${ws._registeredId}`);
      }
    });

    ws.on("error", (err) => console.warn("ESP32错误:", err.message || err));
  } else if (url === "/client") {
    ws._clientId = null;
    ws._token = null;
    ws.on("message", (raw) => {
      try {
        const msg = JSON.parse(raw.toString());
        if (msg.type === "register" && msg.clientId) {
          const clientId = msg.clientId;
          const { token } = msg;
          if (!token || !tokenMap.has(token)) {
            console.log("❌ 非法 token，连接关闭");
            ws.close();
            return;
          }

          if (clientMap.has(clientId)) {
            const oldWs = clientMap.get(clientId);
            console.log(`断开旧浏览器连接: ${clientId}`);
            oldWs.terminate();
          }

          clientMap.set(clientId, ws);
          ws._clientId = clientId;
          console.log(`🌐 浏览器客户端上线: ${clientId}`);
        }
      } catch (e) {
        console.log("非 JSON 消息:", raw.toString());
      }
    });

    ws.on("close", () => {
      if (ws._clientId) {
        clientMap.delete(ws._clientId);
        console.log(`浏览器客户端断开: ${ws._clientId}`);
      }
    });

    ws.on("error", (err) => {
      if (ws._clientId) clientMap.delete(ws._clientId);
      console.warn("浏览器客户端错误:", err.message || err);
    });
  } else {
    console.log("未知路径:", url);
    ws.close();
  }
});

// 验证 token 中间件
function requireToken(req, res) {
  const auth = req.headers.authorization;
  const token = auth?.replace("Bearer ", "");

  if (!token) {
    res.status(401).json({ success: false, message: "缺少 token" });
    return null; // 返回 null 代表不通过
  }

  if (!tokenMap.has(token)) {
    res.status(401).json({ success: false, message: "token 无效或已过期" });
    return null;
  }

  return token; // 返回有效 token
}

// --- HTTP API ---
// HTTP API
app.post("/api/operphone", (req, res) => {
  // 验证 token
  const token = requireToken(req, res);
  if (!token){
    return res
      .status(400)
      .json({ success: false, message: "token 无效或已过期" })
  }
  const { deviceId, cmd, meta } = req.body || {};
  if (!deviceId || !cmd)
    return res
      .status(400)
      .json({ success: false, message: "需要 deviceId 和 cmd" });

  const espWs = espMap.get(deviceId);
  if (!espWs || espWs.readyState !== WebSocket.OPEN)
    return res
      .status(404)
      .json({ success: false, message: `设备 ${deviceId} 不在线` });

  const payload = JSON.stringify({
    type: "cmd",
    deviceId,
    action: cmd,
    meta: meta || {},
    ts: Date.now(),
  });
  try {
    // 发送命令给 ESP32
    espWs.send(payload);
    // 广播给客户端
    broadcastToClients(buildDeviceMessage("server", deviceId, cmd));
    // 返回响应
    res.json({ success: true, message: "命令已发送" });
  } catch (e) {
    console.error("发送命令失败:", e);
    res.status(500).json({ success: false, message: "发送命令失败" });
  }
});

app.get("/api/esp-status", (req, res) => {
  // 验证 token
  const token = requireToken(req, res);
  if (!token){
    return res
      .status(400)
      .json({ success: false, message: "token 无效或已过期" })
  }
  const list = [];
  for (const [deviceId, ws] of espMap.entries())
    list.push({ deviceId, alive: ws.isAlive, readyState: ws.readyState });
  res.json({ count: list.length, devices: list });
});

// 浏览器登录接口
app.post("/api/login", (req, res) => {
  const { username, password } = req.body || {};

  // 简单账号密码校验
  if (username !== "hanser" || password !== "7758521521") {
    return res.status(401).json({ success: false, message: "账号或密码错误" });
  }

  const token = generateToken();
  tokenMap.set(token, username); // token 绑定 clientId（这里用 username）

  res.json({
    success: true,
    token,
    clientId: username,
    message: "登录成功",
  });
});

// 退出登录
app.post("/api/logout", (req, res) => {
  const authHeader = req.headers.authorization;
  const token = authHeader?.replace("Bearer ", "");

  if (!token) {
    return res.status(400).json({
      success: false,
      message: "缺少 token",
    });
  }

  // 判断 token 是否有效
  if (!tokenMap.has(token)) {
    return res.status(401).json({
      success: false,
      message: "token 无效或已退出",
    });
  }

  // 删除 token
  tokenMap.delete(token);

  res.json({
    success: true,
    message: "退出成功",
  });
});

// 心跳检测 (只用 isAlive + lastPong)
const interval = setInterval(() => {
  for (const socket of wss.clients) {
    // 假连接
    const isZombie =
      !socket.isAlive ||
      (socket.lastPong && Date.now() - socket.lastPong > PONG_TIMEOUT);
    if (isZombie) {
      if (socket._registeredId && espMap.has(socket._registeredId)) {
        console.log(`清理死ESP32连接: ${socket._registeredId}`);
        espMap.delete(socket._registeredId);

        broadcastToClients(
          buildDeviceMessage("server", socket._registeredId, "cleanDisconnect")
        );
      }

      if (socket._clientId && clientMap.has(socket._clientId)) {
        console.log(`清理死浏览器连接: ${socket._clientId}`);
        clientMap.delete(socket._clientId);
      }
      // 断开连接
      socket.terminate();
      console.log("连接已终止");
      continue;
    }
    // 标记为待检测
    socket.isAlive = false;
    try {
      socket.ping();
    } catch (e) {
      console.warn("Ping失败:", e.message || e);
    }
  }
}, HEARTBEAT_INTERVAL);

process.on("SIGINT", () => {
  clearInterval(interval);
  console.log("Shutting down server...");
  wss.close();
  server.close(() => process.exit(0));
});

server.listen(PORT, () => {
  console.log(`🚀 Server listening on http://0.0.0.0:${PORT}`);
  console.log(
    `WebSocket paths: ws://<host>:${PORT}/esp  and  ws://<host>:${PORT}/client`
  );
});
