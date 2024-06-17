import Title from '/components/Title'
import Configuration from '/controllers/Configuration'

export const refs = {
  component: null
}

export async function render () {
  await Configuration.load()
  if (!Configuration.title) return

  refs.component = Title.render(Configuration.title)
}
