import './Tint.scss'

import { Component, render } from '@tooooools/ui'
import { writable } from '@tooooools/ui/state'

export default class Tint extends Component {
  static render = (props) => render(<Tint {...props} />).components[0]

  state = {
    color: writable('black')
  }

  template (props, state) {
    return (
      <section
        class='tint'
        style={{
          '--tint-blur': props.blur,
          '--tint-mix-blend-mode': props.mixBlendMode,
          '--tint-color': state.color,
          background: state.background
        }}
      />
    )
  }
}
