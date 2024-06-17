import './strandtest.scss'
import { raf } from '@internet/raf'
import StripLed from '/abstractions/StripLed'
import Configuration from '/controllers/Configuration'

export default async () => {
  await Configuration.load()
  const strip = new StripLed(Configuration.stripLed.ledCount)
  await strip.connect()

  let framecount = 0
  raf.add(dt => {
    framecount++
    strip.fill([0, 0, 0, 0])
    strip.setPixel(framecount + 0, [0, 0, 0, 255], true)
    strip.setPixel(framecount + 1, [0, 0, 255, 0], true)
    strip.setPixel(framecount + 2, [0, 255, 0, 0], true)
    strip.setPixel(framecount + 3, [255, 0, 0, 0], true)
    strip.show()
  })
}
