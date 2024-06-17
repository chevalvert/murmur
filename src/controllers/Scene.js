import { raf } from '@internet/raf'
import { writable } from '@tooooools/ui/state'

import Sequence from '/abstractions/Sequence'
import Video from '/components/Video'

import Configuration from '/controllers/Configuration'
import * as Tint from '/controllers/Tint'

import delay from '/utils/delay'
import RandomPicker from '/utils/random-picker'

let sequencePicker
const loadingNext = writable(false)
const readyToBump = writable(false)

export const refs = {
  transition: null,
  sequence: null
}

export async function render () {
  await Configuration.load()
  sequencePicker = new RandomPicker(Configuration.sequences)

  refs.transition = await Video.render({
    class: 'transition',
    src: Configuration.transition.src,
    loop: true,
    preload: true
  })

  await next({ autoBump: false, instant: true })
  raf.add(tick)

  // Play transition while loading the next sequence
  loadingNext.subscribe(loadingNext => {
    if (loadingNext) refs.transition.play()
    else refs.transition.pause()
  })
}

function tick () {
  if (refs.transition.playing) Tint.disable()
  else Tint.enable()

  if (refs.sequence) refs.sequence.update()
  if (!refs.sequence || refs.sequence.done) next()
}

export async function bump () {
  if (refs.transition.playing) return
  if (!refs.sequence) return
  refs.sequence.bump()
  readyToBump.set(false)
}

export function prepareNextBump (eta, options) {
  if (readyToBump.get()) return
  if (!refs.sequence) return

  readyToBump.set(true)
  return refs.sequence.prepareNextBump(eta, options)
}

export async function next ({ autoBump = true, instant = false } = {}) {
  // Skip if already loading the next sequence
  if (loadingNext.get()) return

  // Destroy previous sequence if any
  if (refs.sequence) {
    raf.remove(refs.sequence.update)
    refs.sequence?.destroy()
  }

  // Start loading the new sequence
  loadingNext.set(true)
  Tint.next()
  refs.sequence = new Sequence(sequencePicker.next())
  await refs.sequence.load()

  if (!instant) await delay(Configuration.transition.duration)

  loadingNext.set(false)
  await delay(10)
  refs.transition.goto(0)
  if (autoBump) bump()
}
