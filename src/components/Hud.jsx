import './Hud.scss'

import { render, Component } from '@tooooools/ui'
import { derived } from '@tooooools/ui/state'
import { map } from 'missing-math'

export default class Hud extends Component {
  static render = (props) => render(<Hud {...props} />).components[0]

  template (props) {
    return (
      <section
        class='hud'
        style={{
          '--hud-font-size': props.fontSize,
          '--hud-padding-x': props.padding[0],
          '--hud-padding-y': props.padding[1]
        }}
      >
        <fieldset>
          <div
            data-label='vol'
            data-symbol={derived(props.microphone.out.trigger, t => t ? '▲' : '△')}
            innerText={
              derived([
                props.microphone.out.volume,
                props.microphone.out.max
              ], ([volume = 0, max = 0]) => {
                const v = Math.round(map(volume, 0, max, 0, 100, true))
                return String(v).padStart(3, '0')
              })
            }
          />
          <div
            data-label='env'
            data-symbol='▽'
            innerText={
              derived([
                props.microphone.raw.volume,
                props.microphone.raw.max
              ], ([volume = 0, max = 0]) => {
                const v = Math.round(map(volume, 0, max, 0, 100, true))
                return String(v).padStart(3, '0')
              })
            }
          />
        </fieldset>

        <div
          data-label='dist'
          data-symbol='◇'
          innerHTML={derived(props.engine.odometer, m => String(m.toExponential(2)).replace(/(\d+)\.(\d+)e\+?(-?.*)/, '$1,$2\u2009<span class="math">×</span>\u200910<sup>$3</sup>'))}
        />
      </section>
    )
  }
}
