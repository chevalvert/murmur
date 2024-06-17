import merge from 'deepmerge'
import dev from '/app.config.dev.json'

const FILE = 'app.config.json'

const configuration = {
  loaded: false,
  load: async function (file = FILE) {
    if (this.loaded) return

    const resp = await fetch(file)
    let data = await resp.json()

    if (import.meta.env.DEV) {
      data = merge(data, dev)
      window.configuration = data
    }

    // Apply root CSS vars
    for (const [prop, value] of Object.entries({
      '--video-offx': data.video?.offset[0],
      '--video-offy': data.video?.offset[1],
      '--video-width': data.video?.width ?? '100vw'
    })) document.documentElement.style.setProperty(prop, value)

    // Mutate this module to attach json props
    for (const key in data) {
      this[key] = Object.freeze(data[key])
    }

    this.loaded = true
  }
}

export { configuration as default }
