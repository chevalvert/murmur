import './scene.scss'
import * as Scene from '/controllers/Scene'

export default async () => {
  await Scene.render()
  Scene.bump()

  window.debug = { Scene }

  // Accelerate on click
  window.addEventListener('click', e => {
    Scene.refs.sequence?.speed(2, { duration: 1000, easing: 'easeInOutSine' })
  })

  window.addEventListener('keypress', e => Scene.bump())
}
