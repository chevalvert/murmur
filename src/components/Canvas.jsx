import './Canvas.scss'

import { render, Component } from '@tooooools/ui'

export default class Canvas extends Component {
  static render = (props) => render(<Canvas {...props} />).components[0]

  template (props) {
    return (
      <canvas
        class={['canvas', props.class]}
        width={props.width}
        height={props.height}
      />
    )
  }

  get width () { return this.base.width }
  get height () { return this.base.height }

  get context () {
    if (!this.mounted) return

    this._context ??= this.base.getContext('2d')
    return this._context
  }

  clear () {
    this.context?.clearRect(0, 0, this.base.width, this.base.height)
  }
}
