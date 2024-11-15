import { clamp } from 'missing-math'

export class Vec2 {
  static ensure (v) { return v instanceof Vec2 ? v : new Vec2(v) }

  clone () { return new Vec2(this.x, this.y) }
  constructor (x = 0, y = 0) {
    this.x = x
    this.y = y
  }

  set mag (mag) { this.mult(mag / this.mag) }
  get magSq () { return this.x * this.x + this.y * this.y }
  get mag () { return Math.sqrt(this.magSq) }

  add (vec, factor = 1) {
    this.x += vec.x * factor
    this.y += vec.y * factor
    return this
  }

  sub (vec, factor = 1) {
    this.x -= vec.x * factor
    this.y -= vec.y * factor
    return this
  }

  mult (factor = 1) {
    this.x *= factor
    this.y *= factor
    return this
  }

  rotate (theta) {
    this.x = this.x * Math.cos(theta) - this.y * Math.sin(theta)
    this.y = this.x * Math.sin(theta) + this.y * Math.cos(theta)
    return this
  }

  clamp (max) {
    if (this.mag > max) this.mag = max
  }

  distSq (vec) { return (vec.x - this.x) * (vec.x - this.x) + (vec.y - this.y) * (vec.y - this.y) }
  dist (vec) { return Math.sqrt(this.distSq(vec)) }
}

export class Particle2 {
  constructor ({
    position = new Vec2(), // px
    acceleration = new Vec2(1, 0), // px/s
    mass = 1e-5, // kg
    trailLength = 1, // px
    lifespan = Number.POSITIVE_INFINITY // milliseconds
  } = {}) {
    this.data = {}

    this.timestamp = Date.now()
    this.initialLifespan = lifespan
    this.lifespan = lifespan
    this.trail = []
    this.trailLength = trailLength
    this.mass = mass

    // Euler
    this.position = position
    this.velocity = new Vec2()
    this.acceleration = new Vec2()
    this.applyForce(acceleration)

    this.update()
  }

  update (dt = 1) {
    this.trails ??= []
    if (this.trails.length > this.trailLength) this.trails.shift()
    else this.trails.push(this.position.clone())

    // Euler
    this.velocity.add(this.acceleration, dt / 1000)
    this.position.add(this.velocity, dt / 1000)
    this.acceleration = new Vec2()

    this.lifespan -= dt
  }

  applyForce (force = new Vec2()) {
    if (!(force instanceof Vec2)) force = new Vec2(force)

    // Euler
    force.mult(1 / this.mass)
    this.acceleration.add(force)
  }

  seek (position = new Vec2(), mass = 1) {
    const force = position.clone().sub(this.position)
    const d = clamp(force.mag, 5, 20)
    force.mult((this.mass * mass) / (d * d))
    this.applyForce(force)
  }

  flee (position = new Vec2(2), power = 1) {
    const force = position.clone().sub(this.position)
    const d = clamp(force.mag, 5, 20)
    force.mult(-1 * d * d * power * this.mass)
    this.applyForce(force)
  }

  // Note that ETA is computed for the current frame specific state, and can
  // vary in time if the particle is applied other forces
  ETA (target) { // in seconds
    target = Vec2.ensure(target)
    return this.position.dist(target) / this.velocity.mag
  }
}

export class Particle1 extends Particle2 {
  constructor (args) {
    args.position = Vec2.ensure(args.position)
    args.acceleration = Vec2.ensure(args.acceleration)
    super(args)
    this.trail = null
    this.pv = this.v
  }

  get v () {
    return Math.ceil(this.position.x)
  }

  get dv () {
    return this.v - this.pv
  }

  get vmax () {
    return this.velocity > 0 ? this.v : this.v + this.trailLength
  }

  update (dt) {
    this.pv = this.v
    super.update(dt)
  }

  absorb (particle, displace = false) {
    this.mass += particle.mass
    if (displace) this.applyForce(particle.velocity.clone().mult(particle.mass))
    this.trailLength = Math.max(this.trailLength, particle.trailLength) + 1
  }
}

export default { Vec2, Particle1, Particle2 }
