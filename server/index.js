const path = require('path')

require('dotenv').config({
  // Handle both pkg and node filesystem for dynamic calls
  // SEE https://github.com/vercel/pkg#snapshot-filesystem
  path: path.resolve(process.pkg
    ? path.dirname(process.execPath)
    : path.join(__dirname, '..'), '.env')
})

const PUBLIC = path.resolve(process.pkg
  ? path.dirname(process.execPath)
  : path.join(__dirname, '..'), 'public')

process.env.NODE_ENV = process.env.NODE_ENV ?? 'production'
process.title = 'murmur-server'

const http = require('http')
const express = require('express')
const logger = require('./logger')
const raspberry = require('./raspberry')
const websocket = require('./websocket')

// Instanciate Rasbperry PI and websocket
const rpi = raspberry(process.env.RPI_ADDRESS, process.env.RPI_PORT)
logger({ color: 'blue', prefix: '[RASPBERRY]' })(process.env.RPI_ADDRESS)
websocket(process.env.WS_PORT, message => {
  try {
    const pixels = JSON.parse(message)
    const payload = pixels.map(p => (p ?? [0, 0, 0, 0]).map(Math.floor)).flat()
    rpi.send(Buffer.from(payload))
  } catch (error) {
    logger({
      color: 'red',
      prefix: '[WEBSOCKET]',
      level: 'error'
    })(error)
  }
})

// Instanciate express server
if (process.env.NODE_ENV === 'production') {
  const app = express()
  const server = http.createServer(app)

  // Log request
  app.use((req, res, next) => {
    logger({ color: 'gray', prefix: '[EXPRESS]' })(req.originalUrl)
    next()
  })

  // Serve static files
  app.use(express.static(path.join(__dirname, '..', 'dist')))
  logger({ color: 'blue', prefix: '[EXPRESS]' })(`Serving public from ${PUBLIC}`)
  app.use(express.static(PUBLIC))

  // Log errors
  app.use((error, req, res, next) => {
    logger({ color: 'red', prefix: '[EXPRESS]', level: 'error' })(error)
    res.status(500).json({ error: error.message })
  })

  // Start HTTP server
  server.listen(process.env.HTTP_PORT, async () => {
    logger({
      color: 'green',
      prefix: '[EXPRESS]'
    })(`HTTP server is up and running on port ${process.env.HTTP_PORT}`)
  })
}
