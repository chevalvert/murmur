import './particles.scss'

import hotkey from 'Hotkeys-js'
import { raf } from '@internet/raf'

import ParticleEngine from '/abstractions/ParticleEngine'
import { Particle1 } from '/abstractions/Particle'
import StripLed from '/abstractions/StripLed'

window.engine = new ParticleEngine({
  size: 300,
  timeFactor: 0.06
})

window.debug = true
window.pause = false

hotkey('w', e => {
  e.stopPropagation()
  window.debug = !window.debug
})

hotkey('p', e => {
  e.stopPropagation()
  window.pause = !window.pause
})

export default async () => {
  const strip = new StripLed(300, { NO_BLIND: true })
  await strip.connect()

  const canvas = document.createElement('canvas')
  canvas.width = 30
  canvas.height = window.engine.size
  const context = canvas.getContext('2d')

  document.body.appendChild(canvas)

  raf.add(tick)

  let currentParticle

  window.addEventListener('keydown', () => {
    window.engine.add(new Particle1({
      acceleration: 5,
      trailLength: 5
    }))
  })

  window.addEventListener('mousedown', () => {
    const dt = Math.random() > 0.5 ? -1 : 1
    currentParticle = window.engine.add(new Particle1({
      position: dt < 0 ? window.engine.size : 0,
      acceleration: dt
    }))
  })
  window.addEventListener('mouseup', () => { currentParticle = null })

  function tick (dt) {
    if (!window.pause) {
      window.engine.update(dt)

      // Increase current trail
      if (currentParticle) {
        currentParticle.trailLength = currentParticle.velocity.magSq < 1
          ? window.engine.size - currentParticle.v
          : currentParticle.v
      }
    }
    render()
  }

  function render () {
    strip.fill([0, 0, 0, 0])
    context.clearRect(0, 0, canvas.width, canvas.height)

    // Flip canvas so that y=0 is at bottom
    context.save()
    context.scale(1, -1)
    context.translate(0, -canvas.height)

    const cx = canvas.width / 2

    if (window.debug) {
      // Render track
      context.strokeStyle = 'rgba(255 255 255 / 20%)'
      context.strokeRect(cx - 1, 0, 3, canvas.height)

      // Render field
      for (let y = 0; y < window.engine.field.length; y++) {
        const o = window.engine.field[y]
        context.fillStyle = `rgba(${o < 0 ? 255 : 0} ${o > 0 ? 255 : 0} 0 / ${Math.abs(o) * 100}%)`

        context.fillRect(cx - 4, y, 2, 1)
        context.fillRect(cx + 3, y, 2, 1)

        pixel(y, [o < 0 ? 255 : 0, o > 0 ? 255 : 0, 0, 0, Math.abs(o)])
      }
    }

    for (const particle of window.engine.particles) {
      const d = Math.sign(particle.velocity.x)
      // Render trail
      for (let i = 0; i < particle.trailLength; i++) {
        const o = (1 - (i / particle.trailLength) ** 2)
        pixel(particle.v - i * d, [0, 0, 0, 255, o])
      }

      if (window.debug) {
        // Render head
        context.fillStyle = 'yellow'
        context.fillRect(cx - 4, particle.v, 9, 1)
      }
    }

    // Render light leak
    // for (let i = 0; i < 30; i++) {
    //   const o = 1 - (i / 30) ** 1.1
    //   pixel(canvas.height - i, [0, 0, 0, 130, o])
    // }

    strip.show()
    context.restore()
  }

  function pixel (i, [r, g, b, w, alpha]) {
    strip.setPixel(i, [r, g, b, w, alpha])
    context.fillStyle = `rgba(${StripLed.rgb([r, g, b, w]).join(' ')} / ${alpha * 100}%)`
    context.fillRect(canvas.width / 2, i, 1, 1)
  }
}
