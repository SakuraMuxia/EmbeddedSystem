// server.js
const express = require('express');
const cors = require('cors');
const http = require("http");
const WebSocket = require("ws");

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });
const PORT = 3000;

// 最新指令缓存
let latestCommand = "";

// 中间件
app.use(cors());
app.use(express.json());
app.use((req, res, next) => {
  console.log('📥 HTTP Request:', req.method, req.url);
  next();
});
// 监听事件，实时打印客户端连接情况
wss.on('connection', (ws, req) => {
  console.log('🌐 有客户端连接 WebSocket:', req.socket.remoteAddress);
  ws.send('Hello from server!');
});

// ====== WebSocket 服务器 ======
app.post("/log", (req, res) => {
  console.log("📡 收到 ESP32 日志:", req.body);
  // 转发给 WebSocket 客户端
  const msg = JSON.stringify(req.body);
  wss.clients.forEach((client) => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(msg || '{}');  // 如果 msg 空，发送空对象
    }
  });
  res.send({ status: "ok" });
});

app.post("/api/operphone", (req, res) => {
  const { command } = req.body;
  if (command === "click" || command === "swipe" || command === "ding" || command === "unlock" || command === "reset"|| command === "rmtask"){
    latestCommand = command; // 缓存最新指令
    console.log(`[指令接收] ${command}`);
    res.json(
      { status: "指令已接收", command }
    )
  }else{
    res.json({ status: "error", message: "未知指令" })
  }
});

// ====== ESP32 轮询获取命令 ======
app.get('/command', (req, res) => {
  res.send(latestCommand);
  if (latestCommand) {
    console.log(`[ESP32 获取] ${latestCommand}`);
  }
  latestCommand = ""; // 读取后清空
});

// ====== 启动服务器 ======
server.listen(PORT, '0.0.0.0',() => {
  console.log(`✅ Server running at http://0.0.0.0:${PORT}`);
});
