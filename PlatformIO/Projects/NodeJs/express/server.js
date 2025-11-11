// server.js
const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');

const app = express();
const PORT = 3000;

// 最新指令缓存
let latestCommand = "";

// 中间件
app.use(cors());
app.use(bodyParser.json());

// ====== 上位端（网页或手机App）发送命令 ======
app.post('/api/operphone', (req, res) => {
  const { command } = req.body;
  
  if (command === "click" || command === "swipe" || command === "ding") {
    latestCommand = command;
    console.log(`[指令接收] ${command}`);
    res.json({ status: "ok", message: `已接收命令: ${command}` });
  } else {
    res.json({ status: "error", message: "未知指令" });
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
app.listen(PORT, () => {
  console.log(`✅ Server running at http://localhost:${PORT}`);
});
