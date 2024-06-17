import anime from 'animejs'
import Video from '/components/Video'

import Configuration from '/controllers/Configuration'
import * as Title from '/controllers/Title'

let video
let timer
let state // sleeping|asleep|waking|awake

export async function load () {
  await Configuration.load()

  video = await Video.render({
    class: 'idle',
    src: Configuration.idle.src,
    loop: true,
    preload: true
  })
}

export function reset () {
  window.clearTimeout(timer)
  timer = window.setTimeout(sleep, Configuration.idle.after)
}

export function sleep ({ force = false, instant = false } = {}) {
  if (state === 'sleeping' && !force) return
  if (state === 'asleep' && !force) return

  state = 'sleeping'

  video.play()
  anime.remove(Title.refs.component.base)
  anime.remove(video.base)
  anime({
    targets: [video.base, Title.refs.component.base],
    opacity: [0, 1],
    ...Configuration.idle.fadeIn,
    duration: instant ? 1 : Configuration.idle.fadeIn.duration,
    complete: () => {
      state = 'asleep'

      Configuration.title.fadeOut && anime({
        targets: Title.refs.component.base,
        opacity: [1, 0],
        ...Configuration.title.fadeOut
      })
    }
  })
}

export function wake () {
  reset()

  if (state === 'waking') return
  if (state === 'awake') return

  state = 'waking'

  anime.remove(Title.refs.component.base)
  anime.remove(video.base)
  anime({
    targets: [video.base, Title.refs.component.base],
    opacity: 0,
    ...Configuration.idle.fadeOut,
    complete: () => {
      state = 'awake'
      video.pause()
    }
  })
}

export default {
  get state () { return state },

  reset,
  load,
  sleep,
  wake
}
