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

  window.addEventListener('click', e => {
    if (sequence.done) return console.log('sequence done')
    sequence.bump()
  })
}
