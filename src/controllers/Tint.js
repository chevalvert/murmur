import { lerp } from 'missing-math'
import { writable } from '@tooooools/ui/state'

import Tint from '/components/Tint'

import Configuration from '/controllers/Configuration'
import RandomPicker from '/utils/random-picker'

const enabled = writable(true)
let tintPicker
export const color = writable(null)

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
}

export function apply (rgbw = [], t = 1) {
  const { led } = color.get() ?? []
  return led
    ? rgbw.map((v, i) => lerp(v, led[i], (led[4] ?? 1) * t))
    : rgbw
}

export function next () {
  color.update(() => {
    const { screen, led, dust } = (tintPicker?.next() ?? [])
    refs.component.state.color.set(screen)
    return { screen, led, dust }
  })
}
