import { clamp } from 'missing-math'
import anime from 'animejs'
import Video from '/components/Video'
import range from '/utils/range'

const increment = (pattern, i) => {
  // Parse and replace printf style %03d pattern
  const [, char, len] = pattern.match(/%(\d)(\d)d/) ?? []
  return pattern.replace(/(%\d\dd)/, String(i).padStart(+len, char))
}

export default class Sequence {
  refs = {
    videos: []
  }

  constructor ({
    steps = 1,
    loopEntry = 30,
    src = ''
  } = {}) {
    this.loopEntry = loopEntry

    this.refs.videos = range(0, steps)
      // Rendering videos in reverse to set correct implicit z-index
      .reverse()
      .map(index => Video.render({ src: increment(src, index + 1) }))
      // Flipping back refs.videos to ensure chronological order
      .reverse()
  }

  async load () {
    for (const video of this.refs.videos) await video.load()
  }

  current
  target = null

  get currentIndex () { return this.refs.videos.indexOf(this.current) }
  get targetIndex () { return this.refs.videos.indexOf(this.target) }

  get done () {
    return this.currentIndex >= this.refs.videos.length - 1 && this.refs.videos[this.refs.videos.length - 1].paused
  }

  get ETA () {
    return this.current
      ? this.current.frameCount - this.current.currentFrame
      : -1
  }

  lastBump = -1

  bump () {
    this.lastBump = Date.now()
    this.target = this.refs.videos[this.currentIndex + 1]
  }

  prepareNextBump (eta, {
    minPlaybackRate = 1,
    maxPlaybackRate = 16,
    secondsBeforeOvershoot = 0
  } = {}) {
    if (!this.current) return

    const { currentTime, duration } = this.current
    const timeLeft = Math.max(0, duration - currentTime - secondsBeforeOvershoot)

    const factor = timeLeft / eta
    this.current.base.playbackRate = clamp(factor, minPlaybackRate, maxPlaybackRate)

    // Send back an adjustment factor if playbackRate cannot be set perfectly,
    // so that we can factor the particle velocity
    return this.current.base.playbackRate / factor
  }

  speed (factor, animation = {}) {
    if (!this.current) return
    anime.remove(this.current.base)
    anime({
      targets: this.current.base,
      playbackRate: factor,
      ...animation
    })
  }

  update = () => {
    // Play the target as soon as possible
    if (this.target && !this.current?.playing) {
      this.current = this.target
      this.current.play()

      this.target = null
      return
    }

    // Loop back
    if (this.current?.currentFrame > this.loopEntry && this.current?.paused) {
      this.current.play(this.loopEntry + 1)
    }
  }

  destroy () {
    this.refs.videos.forEach(video => video?.destroy())
  }
}
