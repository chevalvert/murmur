/* global ResizeObserver */
import './Graph.scss'

import { Component } from '@tooooools/ui'
import { raf } from '@internet/raf'
import { map } from 'missing-math'

const COLORS = [
  '#968ae9',
  '#00d7e9',
  '#ff528c',
  '#ff8c41',
  '#ffe448',
  '#54db87',
  '#8c8890',
  '#f8f1ff'
]

export default class Graph extends Component {
  ctx = null
  frameCount = 0

  state = {
    pvalues: {}
  }

  get range () {
    return this.props.range?.get?.() ?? this.props.range ?? [0, 1]
  }

  template (props) {
    return (
      <section class={['graph', props.class]}>
        <canvas ref={this.ref('canvas')} />
        <ul class='graph__legend'>
          {
            props.values.map((value, index) => (
              <label style={`--color: ${value.color ?? COLORS[index]}`}>
                {value.label}
                <input
                  type='checkbox'
                  checked={!value.disabled}
                  onchange={e => {
                    value.disabled = !e.target.checked
                    this.#onResize()
                  }}
                />
              </label>
            ))
          }
        </ul>
      </section>
    )
  }

  afterMount () {
    this.ctx = this.refs.canvas.getContext('2d')

    this.refs.resizeObserver = new ResizeObserver(this.#onResize)
    this.refs.resizeObserver.observe(this.base)

    raf.add(this.#tick)
  }

  #tick = dt => {
    const step = 2
    const padding = 40
    this.ctx.drawImage(this.refs.canvas, step, 0)

    for (let index = 0; index < this.props.values.length; index++) {
      const {
        signal,
        color = COLORS[index],
        disabled = false,
        range = this.range
      } = this.props.values[index]
      if (disabled) continue

      this.ctx.beginPath()
      this.ctx.strokeStyle = color
      this.ctx.lineJoin = 'round'
      this.ctx.lineCap = 'round'
      this.ctx.lineWidth = step
      this.ctx.moveTo(padding, map(signal.current, range[0], range[1], this.refs.canvas.height - padding, padding))
      this.ctx.lineTo(padding + step, map(this.state.pvalues[index] ?? signal.previous, range[0], range[1], this.refs.canvas.height - padding, padding))
      this.ctx.stroke()

      // Ensure trail is correct even if signal has not changed between frames
      this.state.pvalues[index] = signal.current
    }
  }

  #onResize = () => {
    const { width, height } = this.base.getBoundingClientRect()
    this.refs.canvas.width = width
    this.refs.canvas.height = height
    // this.refs.canvas.style.width = this.refs.canvas.width + 'px'
    // this.refs.canvas.style.height = this.refs.canvas.height + 'px'
    this.ctx.fillRect(0, 0, this.refs.canvas.width, this.refs.canvas.height)
  }

  beforeDestroy () {
    raf.remove(this.#tick)
  }
}
