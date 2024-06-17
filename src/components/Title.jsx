import './Title.scss'

import { Component, render } from '@tooooools/ui'

export default class Title extends Component {
  static render = (props) => render(<Title {...props} />).components[0]

  template (props) {
    return (
      <section
        class='title'
        innerHTML={props.text}
        style={{
          '--title-font-size': props.fontSize,
          '--title-v-weight': props.vWeight,
          '--title-v-italic': props.vItalic,
          '--title-color': props.color,
          '--title-line-height': props.lineHeight,
          '--title-offx': props.offset[0],
          '--title-offy': props.offset[1]
        }}
      />
    )
  }
}
