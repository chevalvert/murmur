import { writable } from '@tooooools/ui/state'

export default class ParticleEngine {
  constructor ({
    size = Number.POSITIVE_INFINITY
  } = {}) {
    this.size = size

    this.particles = []
    this.lastTouch = writable({ timestamp: -1 })
    this.odometer = writable(0)
    this.field = new Array(size).fill(0)
  }

  add (p) {
    this.particles.push(p)
    return p
  }

  update (dt) {
    const garbage = []

    this.particles.sort((a, b) => b.position - a.position)

    for (const particle of this.particles) {
      // Update
      particle.update(dt)
      this.odometer.update(v => isFinite(particle.dv) ? v + (particle.dv / 60) : v) // In real-life meters

      // Cheap way to emit an event on particle exiting
      if (particle.v > this.size && this.lastTouch.current?.timestamp < particle.timestamp) {
        this.lastTouch.set(particle)
      }

      // Too slow or not moving
      if (Math.abs(particle.velocity.magSq) <= 1e-3) {
        garbage.push(particle)
      }

      // OOB
      if ((particle.velocity.magSq > 0 && (particle.v < 0 || particle.v - particle.trailLength > this.size)) ||
        (particle.velocity < 0 && (particle.v + particle.trailLength < 0 || particle.v > this.size))) {
        garbage.push(particle)
      }

      // End-of-life
      if (particle.lifespan <= 0) {
        garbage.push(particle)
      }
    }

    // Cleanup dead particles
    for (const p of garbage) {
      this.particles.splice(this.particles.indexOf(p), 1)
    }
  }
}
