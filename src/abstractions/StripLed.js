import WebSocket from 'reconnectingwebsocket'
import { writable } from '@tooooools/ui/state'
import { lerp } from 'missing-math'

const URL = 'ws://localhost:1337'
const BLACK = [0, 0, 0, 0]

export default class StripLed {
  constructor (ledCount, {
    NO_BLIND = false // Prevent bright lights
  } = {}) {
    this.ws = new WebSocket(URL, null, { automaticOpen: false })
    this.$connected = writable(false)

    this.ledCount = ledCount
    this.NO_BLIND = NO_BLIND
    this.pixels = new Array(this.ledCount).fill([0, 0, 0, 0])
  }

  async connect () {
    if (this.$connected.get()) return true

    return new Promise(resolve => {
      this.$connected.set(true)
      this.ws.onopen = () => resolve(true)
      this.ws.onerror = err => console.warn(err)
      this.ws.open()
    })
  }

  clear () {
    this.fill()
  }

  fill (color = BLACK) {
    this.pixels = new Array(this.ledCount).fill(color)
  }

  setPixel (i, [r, g, b, w, a = 1], warp = false) {
    if (warp) i = i % (this.pixels.length - 1)

    this.pixels[i] ??= [0, 0, 0, 0]
    this.pixels[i] = [
      lerp(this.pixels[i][0], r, a),
      lerp(this.pixels[i][1], g, a),
      lerp(this.pixels[i][2], b, a),
      lerp(this.pixels[i][3], w, a)
    ]
  }

  render () {
    if (!this.canvas) {
      this.canvas = document.createElement('canvas')
      this.canvas.classList.add('stripled')
      this.canvas.width = 3
      this.canvas.height = this.ledCount
      this.canvas.context = this.canvas.getContext('2d')
      document.body.appendChild(this.canvas)
    }

    for (let index = 0; index < this.pixels.length; index++) {
      const [r, g, b] = StripLed.rgb(this.pixels[index])
      this.canvas.context.fillStyle = `rgb(${r}, ${g}, ${b})`
      this.canvas.context.fillRect(0, (this.canvas.height - index), this.canvas.width, 1)
    }
  }

  show (framecount) {
    if (!this.$connected.get()) return false

    if (this.NO_BLIND) {
      for (let index = 0; index < this.pixels.length; index++) {
        this.pixels[index] = (this.pixels[index] ?? BLACK).map(v => Math.min(v, 5))
      }
    }

    this.ws.send(JSON.stringify(this.pixels))
    return true
  }

  // SEE https://stackoverflow.com/questions/51785689/what-is-an-effective-way-to-map-rgbw-pixel-data-to-rgb-for-a-visualisation
  static rgb ([r, g, b, w] = [0, 0, 0, 0]) {
    const w2 = (1.0 - (1.0 - w / 255) * (1.0 - w / 255))
    const alpha = 0.6 * w2

    return [
      ((1.0 - alpha) * r / 255 + alpha) * 255,
      ((1.0 - alpha) * g / 255 + alpha) * 255,
      ((1.0 - alpha) * b / 255 + alpha) * 255
    ]
  }
}
