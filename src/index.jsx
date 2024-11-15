import '/index.scss'

import { raf } from '@internet/raf'
import { clamp } from 'missing-math'
import Microphone from '/abstractions/Microphone'
import { Particle1 } from '/abstractions/Particle'
import ParticleEngine from '/abstractions/ParticleEngine'
import StripLed from '/abstractions/StripLed'

import Configuration from '/controllers/Configuration'
import * as Dust from '/controllers/Dust'
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
  await Sound.load(...Object.values(Configuration.sounds))
  await Idle.load()

  // Render everything
  await Tint.render()
  await Scene.render()
  await Dust.render()
  await Title.render()
  await Hud.render({ microphone, engine })
  await Mask.render()

  // Init
  document.body.dataset.state = 'ready'
  Sound.play(Configuration.sounds.ambient)
  Idle.sleep({ instant: true })

  // Spawn led particles
  let held = null
  microphone.out.trigger.subscribe(t => {
    if (t < 1) held = null
    else {
      held = engine.add(new Particle1({
        acceleration: Configuration.particles.acceleration,
        trailLength: Configuration.particles.minLength ?? 1
      }))
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

  function touch (ledParticle, { forceBump = false } = {}) {
    const dustParams = {
      ...Configuration.dust,
      ...Scene.refs.sequence?.data.dust ?? {},
      color: Tint.color.get()?.dust,
      trailLength: Math.max(2, ledParticle?.trailLength ?? 0)
    }

    // Bump immediately if no sequence is playing
    if (!Scene.refs.sequence.current) {
      Sound.play(Configuration.sounds.events)
      Dust.bump(dustParams)
      Scene.bump()
      return
    }

    const canBump = !Scene.refs.sequence.willBump &&
      ledParticle?.trailLength >= Configuration.particles.streamThreshold

    if (canBump) {
      // Emit a stream of particles
      const stream = {
        ...Configuration.stream,
        ...Scene.refs.sequence.data?.stream ?? {}
      }

      const streamParams = {
        ...dustParams,
        ...stream.dust ?? {}
      }

      // Adjust scene time to ensure sync with the future bump
      const { timeLeft } = Scene.prepareNextBump(dustParams.lifespan / 1000, Configuration.video) ?? 1

      for (let i = 0; i < stream.quantity; i++) {
        const d = i * ((timeLeft * 1000) / stream.quantity)
        window.setTimeout(() => Dust.bump(streamParams), d)
      }

      Sound.play(Configuration.sounds.stream)

      // Bump Scene when particle dies
      window.setTimeout(Scene.bump, dustParams.lifespan)
    } else {
      // Send a single dust particle
      Sound.play(Configuration.sounds.events)
      Dust.bump(dustParams)
    }
  }

  function update (dt) {
    // Stretch currently held particle
    if (held) held.trailLength = clamp(held.v, held.trailLength, Configuration.particles.maxLength ?? 999)

    Dust.update(dt)
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
      const d = Math.sign(particle.velocity.x)
      for (let i = 0; i < particle.trailLength; i++) {
        let o = (1 - (i / particle.trailLength) ** 2)
        o *= (Math.min(particle.lifespan, 100) / 100) ** 0.5
        const t = clamp((particle.v / engine.size) ** 3, 0, 1)
        strip.setPixel(particle.v - i * d, [...Tint.apply([0, 0, 0, 255], t), o])
      }
    }

    if (Flags.get('DISPLAY_STRIPLED')) strip.render()
    strip.show()
  }
}
