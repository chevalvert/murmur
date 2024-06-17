/* eslint-disable prefer-promise-reject-errors */
import './Video.scss'

import { Component, render } from '@tooooools/ui'
import { writable } from '@tooooools/ui/state'

export default class Video extends Component {
  state = {
    loaded: writable(false)
  }

  static render (props) {
    const video = render(<Video {...props} />).components[0]
    return props.preload
      ? video.load().then(() => video)
      : video
  }

  /**
   * IMPORTANT
   * <Video> time unit = frame
   * <video> time unit = second
   */

  get frameRate () { return this.props.frameRate ?? 30 }

  get currentTime () { return this.base.currentTime }
  get duration () { return this.base.duration }

  get frameCount () { return Math.floor(this.base.duration * this.frameRate) }
  get currentFrame () { return Math.floor(this.base.currentTime * this.frameRate) }
  set currentFrame (f) { this.base.currentTime = f / this.frameRate }

  template (props, state) {
    return (
      <video
        muted
        loop={props.loop}
        preload='metadata'
        controls={props.controls}
        class={[props.class, { 'is-loaded': state.loaded }]}
      >
        <source src={props.src} type='video/mp4' />
      </video>
    )
  }

  #onLoadedMetadata = () => this.state.loaded.set(true)

  afterMount () {
    this.base.addEventListener('loadedmetadata', this.#onLoadedMetadata)
  }

  async load () {
    if (this.state.loaded.get()) return true

    return new Promise(resolve => {
      this.state.loaded.subscribe(resolve)
      this.base.load()
    })
  }

  get paused () { return this.base.paused }
  get playing () { return !this.base.paused }

  play (from = 0) {
    this.currentFrame = from
    this.raise()
    return this.base.play()
  }

  pause () { this.base.pause() }
  goto (to = 0) { this.currentFrame = to }

  raise () {
    for (const video of document.querySelectorAll('video')) {
      video.classList.toggle('is-raised', video === this.base)
    }
  }

  beforeDestroy () {
    this.base.removeEventListener('loadedmetadata', this.#onLoadedMetadata)
  }
}
