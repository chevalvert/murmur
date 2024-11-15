import './dust.scss'

import { raf } from '@internet/raf'
import Video from '/components/Video'

import * as Tint from '/controllers/Tint'
import * as Dust from '/controllers/Dust'

export default async () => {
  await Tint.render()
  ;(await Video.render({
    src: 'videos/c/001.mp4',
    loop: true,
    preload: true
  })).play()

  await Dust.render()

  window.debug = Dust.refs

  raf.add(Dust.update)
  window.setInterval(() =>
    Dust.bump({
      trailLength: 1000,
      lifespan: 1000
    })
  , 1000)

  window.addEventListener('click', e => {
    for (let i = 0; i < Math.random() * 10; i++) {
      window.setTimeout(() => Dust.bump({ tint: Tint.color.get() }), i * 50)
    }
  })
}
