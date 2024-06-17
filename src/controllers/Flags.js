const flags = new Map(Array.from(new URL(window.location).searchParams))

function update () {
  // Update URL without reloading
  const url = new URL(window.location)
  for (const [flag, value] of flags) {
    url.searchParams.set(flag, value)
  }
  window.history.pushState(null, '', url)
  document.body.classList.toggle('has-flags', Array.from(flags).length)
}

export function set (flag, value, force = false) {
  flag = flag.toLowerCase()

  const params = new URL(window.location).searchParams
  const param = params.get(flag)

  const parse = v => {
    switch (typeof value) {
      case 'boolean': return v === 'true'
      case 'string': return String(v)
      case 'number': return Number(v)
      default: return v
    }
  }

  if (!force && params.has(flag)) flags.set(flag, parse(param))
  else flags.set(flag, value)

  update()
}

export function get (flag) {
  return flags.get(flag.toLowerCase())
}
