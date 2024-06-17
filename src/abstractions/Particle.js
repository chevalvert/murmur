export default class Particle {
  constructor ({
    position = 0, // px
    acceleration = 1, // m/s
    mass = 0.01, // kg
    trail = 1, // px
    color = [0, 0, 0, 255], // RGBW
    lifespan = Number.POSITIVE_INFINITY // frames
  } = {}) {
    this.timestamp = Date.now()
    this.lifespan = lifespan
    this.mass = mass
    this.trail = trail
    this.color = color

    // Euler
    this.pintpos = position
    this.position = position
    this.velocity = 0
    this.acceleration = 0
    this.applyForce(acceleration)

    this.update()
  }

  get intpos () {
    return Math.ceil(this.position)
  }

  get ymax () {
    return this.velocity > 0 ? this.intpos : this.intpos + this.trail
  }

  get dintpos () {
    return this.intpos - this.pintpos
  }

  update (dt = 1) {
    this.pintpos = this.intpos

    // Euler
    this.velocity += this.acceleration
    this.position += this.velocity * (dt / 1000)
    this.acceleration = 0

    this.lifespan--
  }

  applyForce (force = 0) {
    // Euler
    this.acceleration += force / this.mass
  }

  absorb (particle, displace = false) {
    this.mass += particle.mass
    if (displace) this.applyForce(particle.velocity * particle.mass)
    this.trail = Math.max(this.trail, particle.trail) + 1
  }

  // Note that ETA is computed for the current frame specific state, and can
  // vary in time if the particle is applied other forces
  ETA (target) {
    const dist = Math.abs(target - this.position)
    return dist / this.velocity
  }
}
