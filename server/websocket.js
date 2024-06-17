const { WebSocketServer } = require('ws')

module.exports = (port, callback) => {
  // From local web browser via websocket
  const wss = new WebSocketServer({ port: process.env.WS_PORT })
  wss.on('connection', async (ws, req) => {
    ws.on('message', callback)
  })
}
