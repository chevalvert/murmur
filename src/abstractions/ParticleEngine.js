import { writable } from '@tooooools/ui/state'

export default class ParticleEngine {
  constructor ({
    size = Number.POSITIVE_INFINITY
  } = {}) {
    this.size = size

    this.particles = []
    this.lastTouch = writable(-1)
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

    for (let index = 0; index < this.particles.length; index++) {
      const curr = this.particles[index]

      // Update current particle
      curr.applyForce(this.field[curr.intpos])
      curr.update(dt)

      this.odometer.update(v => v + (curr.dintpos / 60)) // In real-life meters

      // Cheap way to emit an event on particle exiting
      if (curr.intpos > this.size && this.lastTouch.current < curr.timestamp) {
        this.lastTouch.set(curr.timestamp)
      }

      // Too slow or not moving
      if (Math.abs(curr.velocity) <= 1e-3) {
        garbage.push(curr)
      }

      // OOB
      if ((curr.velocity > 0 && (curr.intpos < 0 || curr.intpos - curr.trail > this.size)) ||
        (curr.velocity < 0 && (curr.intpos + curr.trail < 0 || curr.intpos > this.size))) {
        garbage.push(curr)
      }

      // End-of-life
      if (curr.lifespan <= 0) {
        garbage.push(curr)
      }
    }

    // Cleanup dead particles
    for (const p of garbage) {
      this.particles.splice(this.particles.indexOf(p), 1)
    }
  }
}
