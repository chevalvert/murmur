import anime from 'animejs'
import { lerp } from 'missing-math'
import { writable } from '@tooooools/ui/state'

import Tint from '/components/Tint'

import Configuration from '/controllers/Configuration'
import RandomPicker from '/utils/random-picker'

const enabled = writable(true)
const color = writable(null)
let tintPicker

export const refs = {
  component: null
}

export async function render () {
  await Configuration.load()
  if (!Configuration.tint) return

  tintPicker = new RandomPicker(Configuration.tint.colors ?? [])
  refs.component = Tint.render(Configuration.tint)

  next()
  enable()
}

export function enable () {
  if (enabled.get()) return
  enabled.set(true)
  next()
}

export function disable () {
  if (!enabled.get()) return
  enabled.set(false)
  color.set(null)
  anime.remove()
}

export async function bump () {
  anime.remove()
  await anime({
    targets: refs.component.base,
    keyframes: [
      { scaleX: 0.9, scaleY: 0.9, duration: 600 },
      { scaleX: 1, scaleY: 1, duration: 300 }
    ],
    easing: 'easeOutExpo'
  }).finished
}

export function apply (rgbw = [], t = 1) {
  const [, led] = color.get() ?? []
  return led
    ? rgbw.map((v, i) => lerp(v, led[i], (led[4] ?? 1) * t))
    : rgbw
}

export function next () {
  color.update(() => {
    const [css, led] = (tintPicker?.next() ?? [])
    refs.component.state.color.set(css)
    return [css, led]
  })
}
