/* global __VERSION__ */
import '/test/test.scss'
import { render } from '@tooooools/ui'

// Basic routing for test/**

const test = window.location.pathname?.replace(/^\//, '')
document.body.dataset.test = test

if (!test) {
  render(<h1>Murmur@{__VERSION__}</h1>)
  for (const path in import.meta.glob('./**/*.jsx', { query: '?url' })) {
    const name = (/^\.\/(.*?)\//.exec(path) ?? [])[1]
    render(<a href={name}>{name}</a>)
  }
} else {
  console.log(`%c[${__VERSION__}] ${test}`, 'background-color: #fef1be; padding: 5px; border-radius: 3px; font-weight: bold')
  ;(await import(`./${test}/${test}.jsx`)).default()
}
