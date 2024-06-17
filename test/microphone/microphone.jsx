import './microphone.scss'
import { render } from '@tooooools/ui'

import Microphone from '/abstractions/Microphone'
import Graph from '/components/Graph'
import * as Flags from '/controllers/Flags'

Flags.set('MICROPHONE_THRESHOLD', 1.0)
Flags.set('MOCKED_MICROPHONE', false)

export default async () => {
  const microphone = new Microphone({
    threshold: +Flags.get('MICROPHONE_THRESHOLD'),
    MOCKED: Flags.get('MOCKED_MICROPHONE')
  })
  await microphone.connect()

  render(
    <>
      <Graph
        range={[0, 100]}
        values={[
          { label: 'raw', signal: microphone.raw.volume },
          { label: 'max', signal: microphone.raw.max },
          { label: 'raw threshold', signal: microphone.raw.threshold }
        ]}
      />
      <Graph
        range={[0, 100]}
        values={[
          { label: 'out', signal: microphone.out.volume },
          { label: 'max', signal: microphone.out.max },
          { label: 'out threshold', signal: microphone.out.threshold }
        ]}
      />
      <Graph
        values={[
          { label: 'raw trigger', signal: microphone.raw.trigger },
          { label: 'out trigger', signal: microphone.out.trigger }
        ]}
      />
      <Graph
        range={[60, 10_000]}
        values={[
          { label: 'raw pitch', signal: microphone.raw.pitch },
          { label: 'out pitch', signal: microphone.out.pitch }
        ]}
      />
    </>
  )
}
