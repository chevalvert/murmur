const UDP = require('dgram')

module.exports = (address, port) => {
  const raspberry = UDP.createSocket('udp4')
  raspberry.on('error', console.error)

  return {
    send: buffer => raspberry.send(buffer, port, address)
  }
}
