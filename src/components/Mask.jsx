import './Mask.scss'

import { Component, render } from '@tooooools/ui'

export default class Mask extends Component {
  static render = (props) => render(<Mask {...props} />).components[0]

  template (props) {
    return (
      <section
        class='mask'
        innerHTML={props.text}
        style={{
          '--mask-x': props.offset[0],
          '--mask-y': props.offset[1],
          '--mask-radius': props.radius,
          '--mask-blur': props.blur
        }}
      />
    )
  }
}
