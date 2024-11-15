import './sequence.scss'

import { raf } from '@internet/raf'
import Sequence from '/abstractions/Sequence'
import Configuration from '/controllers/Configuration'

export default async () => {
  await Configuration.load()

  const sequence = new Sequence(Configuration.sequences[0])
  window.debug = { sequence }

  await sequence.load()
  raf.add(sequence.update)
  sequence.bump()

  window.addEventListener('click', e => {
    if (sequence.done) return console.log('sequence done')

    const delay = 3000
    console.log(`Next bump in ${delay}ms`)
    sequence.prepareNextBump(delay / 1000, { secondsBeforeOvershoot: 0.25 })

    window.setTimeout(() => {
      console.log(Date.now(), 'bump')
      sequence.bump()
    }, delay)
  })
}
