import { Howl } from 'howler'
import randomOf from '/utils/array-random'

const sounds = new Map()

export async function load (...sources) {
  for (const { src, ...options } of sources.flat()) {
    const h = new Howl({ src: [src], ...options })
    await h.load()
    sounds.set(src, h)
  }
}

export function play (asset) {
  if (Array.isArray(asset)) asset = randomOf(asset)
  sounds.get(asset.src)?.play()
}
