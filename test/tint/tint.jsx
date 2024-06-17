import './tint.scss'

import { raf } from '@internet/raf'

import Particle from '/abstractions/Particle'
import ParticleEngine from '/abstractions/ParticleEngine'
import Sequence from '/abstractions/Sequence'
import StripLed from '/abstractions/StripLed'

import Configuration from '/controllers/Configuration'
import * as Tint from '/controllers/Tint'

export default async () => {
  await Configuration.load()
  await Tint.render()

  const strip = new StripLed(Configuration.stripLed.ledCount)
  const sequence = new Sequence(Configuration.sequences[0])
  const engine = new ParticleEngine({ size: Configuration.stripLed.ledCount })

  await strip.connect()
  await sequence.load()
  sequence.bump()

  raf.add(update)
  raf.add(draw)

  window.addEventListener('keypress', Tint.next)

  let held = null
  window.addEventListener('mouseup', () => { held = null })
  window.addEventListener('mousedown', () => {
    held = engine.add(new Particle({
      acceleration: Configuration.particles.acceleration,
      trail: Configuration.particles.minLength
    }))
  })

  function update (dt) {
    if (held) held.trail = held.intpos
    engine.update(dt)
    sequence.update()
  }

  function draw (dt) {
    strip.clear()

    // Render particles
    for (const particle of engine.particles) {
      const d = Math.sign(particle.velocity)
      for (let i = 0; i < particle.trail; i++) {
        let o = (1 - (i / particle.trail) ** 2)
        o *= (Math.min(particle.lifespan, 100) / 100) ** 0.5
        const t = (particle.intpos / engine.size) ** 3
        strip.setPixel(particle.intpos - i * d, [...Tint.apply(particle.color, t), o])
      }
    }

    strip.render()
    strip.show()
  }
}
