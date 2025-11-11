const http = require("http");
const WebSocket = require("ws");

const server = http.createServer();
const wss = new WebSocket.Server({ server });

wss.on('connection', (ws, req) => {
  console.log('🌐 WebSocket connected:', req.socket.remoteAddress);
  ws.send('Hello from server!');
});

server.listen(3002, '0.0.0.0', () => console.log('Server running on 0.0.0.0:3000'));