import Mask from '/components/Mask'
import Configuration from '/controllers/Configuration'

export const refs = {
  base: null
}

export async function render () {
  await Configuration.load()
  if (!Configuration.mask) return

  refs.base = Mask.render(Configuration.mask)
}
