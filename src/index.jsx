import '/index.scss'

import { raf } from '@internet/raf'
import Microphone from '/abstractions/Microphone'
import Particle from '/abstractions/Particle'
import ParticleEngine from '/abstractions/ParticleEngine'
import StripLed from '/abstractions/StripLed'

import Configuration from '/controllers/Configuration'
import * as Flags from '/controllers/Flags'
import * as Hud from '/controllers/Hud'
import * as Idle from '/controllers/Idle'
import * as Mask from '/controllers/Mask'
import * as Scene from '/controllers/Scene'
import * as Sound from '/controllers/Sound'
import * as Tint from '/controllers/Tint'
import * as Title from '/controllers/Title'

;(async () => {
  await Configuration.load()

  if (Configuration.debug) {
    console.log('%cDEBUG mode enabled', 'background-color: #ff528c; color: black; padding: 8px; border-radius: 5px; font-weight: bold')
    Flags.set('MOCKED_MICROPHONE', true)
    Flags.set('NO_BLIND', true)
    Flags.set('DISPLAY_STRIPLED', true)
  }

  if (Flags.get('NO_CLICK')) setup()
  else window.addEventListener('click', setup, { once: true })
})()

async function setup () {
  document.body.dataset.state = 'loading'

  const engine = new ParticleEngine({ size: Configuration.stripLed.ledCount })
  const microphone = new Microphone({ MOCKED: Flags.get('MOCKED_MICROPHONE'), treshold: Configuration.microphone.threshold })
  const strip = new StripLed(Configuration.stripLed.ledCount, { NO_BLIND: Flags.get('NO_BLIND') })

  // Load everything
  await microphone.connect()
  await strip.connect()
  await Sound.load(Configuration.sounds.ambient, Configuration.sounds.events)
  await Idle.load(Configuration.idle)

  // Render everything
  await Tint.render()
  await Scene.render()
  await Title.render()
  await Hud.render({ microphone, engine })
  await Mask.render()

  // Init
  document.body.dataset.state = 'ready'
  Sound.play(Configuration.sounds.ambient)
  Idle.sleep({ instant: true })

  // Spawn particles
  let held = null
  microphone.out.trigger.subscribe(t => {
    if (t < 1) held = null
    else {
      held = engine.add(new Particle({
        acceleration: Configuration.particles.acceleration,
        trail: Configuration.particles.minLength ?? 1
      }))

      // Adjust scene time to ensure sync with the future bump
      const feedback = Scene.prepareNextBump(held.ETA(engine.size), Configuration.video)
      // Adjust back particle velocity if needed
      held.velocity *= feedback ?? 1
    }
  })

  // Run
  raf.add(update)
  raf.add(draw)
  engine.lastTouch.subscribe(touch)

  // Expose instances and controllers to simplify debug, even in production
  window.debug = {
    microphone,
    strip,
    engine,
    Sound,
    Scene,
    Idle
  }

  function touch () {
    if (Date.now() - Scene.refs.sequence.lastBump >= Configuration.particles.minBumpInterval) Scene.bump()
    Tint.bump()
    Sound.play(Configuration.sounds.events)
  }

  function update (dt) {
    // Stretch currently held particle
    if (held) held.trail = held.intpos

    engine.update(dt)
    if (engine.particles.length || Scene.refs.sequence.playing) Idle.wake()
  }

  function draw (dt) {
    strip.clear()

    // Inside the object based on microphone raw input
    const c = Math.floor(Configuration.stripLed.inside.length * Math.min(1, (microphone.out.max.current / microphone.out.threshold.current)))
    for (let i = 0; i < c; i++) {
      strip.setPixel(i, [...Configuration.stripLed.inside.color, (i / c)])
    }

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

    if (Flags.get('DISPLAY_STRIPLED')) strip.render()
    strip.show()
  }
}
