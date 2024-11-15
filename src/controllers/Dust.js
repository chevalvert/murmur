import Configuration from '/controllers/Configuration'

import Canvas from '/components/Canvas'
import { clamp, lerp, radians, random, normalize } from 'missing-math'
import { Particle2, Vec2 } from '/abstractions/Particle'

const MAX_PARTICLES = 1_000
const MAX_LIFESPAN = 10_000 // ms

export const refs = {
  particles: [],
  canvas: null
}

export async function render () {
  await Configuration.load()
  if (!Configuration.dust) return

  refs.canvas = Canvas.render({
    ...Configuration.dust
  })

  refs.canvas.context.lineJoin = 'round'
  refs.canvas.context.lineCap = 'round'
}

export function update (dt) {
  if (!refs.canvas) return
  refs.canvas.clear()

  const garbage = []
  const center = new Vec2(refs.canvas.width / 2, refs.canvas.height / 2)

  for (const particle of refs.particles) {
    const t = 1 - normalize(particle.lifespan, 0, particle.initialLifespan, true)

    // Attract to center
    particle.seek(center, 1e4)

    // Rotate a little
    if (particle.lifespan < particle.initialLifespan * 0.9 && particle.position.distSq(center) < particle.data.diffractionRadius ** 2) {
      particle.velocity.rotate(radians(Math.sin(particle.timestamp) * particle.data.diffractionAmount))
    }

    particle.update(dt)

    // Start destroying trail when reaching end of life
    if (particle.lifespan < 0) particle.trailLength = 0

    // Render
    refs.canvas.context.beginPath()
    refs.canvas.context.lineWidth = Math.max(1, lerp(particle.data.minThickness, particle.data.maxThickness, (1 - t) ** 3))
    refs.canvas.context.strokeStyle = `rgba(
      ${particle.data.color[0]},
      ${particle.data.color[1]},
      ${particle.data.color[2]},
      ${particle.data.color[3] || 1}
    )`
    for (let i = 0; i < particle.trails.length; i++) {
      const pos = particle.trails[i]
      if (!pos) continue
      refs.canvas.context[i ? 'lineTo' : 'moveTo'](pos.x, pos.y)
    }
    refs.canvas.context.stroke()

    // Garbage collection
    if (particle.velocity.magSq < 1) garbage.push(particle)
    if (particle.position.x < 0 || particle.position.y < 0 || particle.position.x > refs.canvas.width || particle.position.y > refs.canvas.height) garbage.push(particle)
    if (particle.lifespan < 0 && particle.trails.length <= 1) garbage.push(particle)
  }

  // Clean up
  for (let i = 0; i < refs.particles.length - MAX_PARTICLES; i++) garbage.push(refs.particles[i])
  for (const p of garbage) refs.particles.splice(refs.particles.indexOf(p), 1)
}

export function bump ({
  color = [255, 255, 255],
  spread = 30, // degrees
  thrust = [15, 25], // px/s
  lifespan = 3000, // milliseconds
  trailLength = random(100), // frames
  diffractionAmount = 10, // degrees
  diffractionRadius = 200, // px
  minThickness = 1, // px
  maxThickness = 50 // px
} = {}) {
  // Randomize parameters if given a arrays
  const handleRange = value => Array.isArray(value) ? random(value[0], value[1]) : value
  spread = handleRange(spread)
  thrust = handleRange(thrust)
  lifespan = handleRange(lifespan)
  trailLength = handleRange(trailLength)
  diffractionAmount = handleRange(diffractionAmount)
  diffractionRadius = handleRange(diffractionRadius)
  minThickness = handleRange(minThickness)
  maxThickness = handleRange(maxThickness)

  // Create particle
  const theta = radians(90 + random(-spread / 2, spread / 2))
  const particle = new Particle2({
    position: new Vec2(refs.canvas.width / 2, refs.canvas.height / 2),
    trailLength,
    lifespan: clamp(lifespan, 0, MAX_LIFESPAN),
    acceleration: (new Vec2(Math.cos(-theta), Math.sin(-theta))).mult(thrust)
  })

  // Attach arbitrary data to the created particle
  particle.data.color = color
  particle.data.diffractionAmount = diffractionAmount
  particle.data.diffractionRadius = diffractionRadius
  particle.data.minThickness = minThickness
  particle.data.maxThickness = maxThickness

  refs.particles.push(particle)
  return particle
}
