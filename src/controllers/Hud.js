import Hud from '/components/Hud'
import Configuration from '/controllers/Configuration'

export const refs = {
  base: null
}

export async function render ({ microphone, engine }) {
  await Configuration.load()
  if (!Configuration.hud) return

  refs.base = Hud.render({
    ...Configuration.hud,
    microphone,
    engine
  })
}
