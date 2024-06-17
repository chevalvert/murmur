/* global AudioContext, AnalyserNode, AudioWorkletNode */
import { writable } from '@tooooools/ui/state'
import { PitchDetector } from 'pitchy'
import NoiseGate from '/utils/noise-gate'
import AudioWorklet from './AudioWorkletProcessor.js?url'
import average from '/utils/array-average'

const MICROPHONE_OPTIONS = {
  audio: {
    echoCancellation: true,
    noiseSuppression: true,
    autoGainControl: false
  }
}

export default class Microphone {
  constructor ({
    threshold = 1,
    MOCKED = false
  } = {}) {
    this.audioContext = new AudioContext()
    this.threshold = threshold
    this.MOCKED = MOCKED

    // Will be populate after Microphone.connect
    this.raw = {}
    this.out = {}
  }

  async connect () {
    if (this.MOCKED) return this.#mock()

    const stream = await navigator.mediaDevices.getUserMedia(MICROPHONE_OPTIONS)

    const input = this.audioContext.createMediaStreamSource(stream)
    const output = this.audioContext.createGain()
    const noiseGate = new NoiseGate(this.audioContext)

    input.connect(noiseGate)
    noiseGate.connect(output)

    this.raw = await this.#createAnalyze(input)
    this.out = await this.#createAnalyze(output)
  }

  #mock () {
    this.raw = {
      volume: writable(0),
      min: writable(0),
      max: writable(0),
      threshold: writable(0),
      trigger: writable(0),
      pitch: writable(0)
    }

    this.out = {
      volume: writable(0),
      min: writable(0),
      max: writable(0),
      threshold: writable(0),
      trigger: writable(0),
      pitch: writable(0)
    }

    window.addEventListener('mousedown', e => this.out.trigger.set(1))
    window.addEventListener('mouseup', e => this.out.trigger.set(0))
  }

  async #createAnalyze (source) {
    const volume = writable(0)
    const min = writable(0)
    const max = writable(0)
    const threshold = writable(0)
    const trigger = writable(0)
    const pitch = writable(0)

    const analyser = new AnalyserNode(this.audioContext, {
      fftSize: 1024,
      smoothingTimeConstant: 0.6
    })

    await this.audioContext.audioWorklet.addModule(AudioWorklet)
    const processor = new AudioWorkletNode(this.audioContext, 'processor')

    const detector = PitchDetector.forFloat32Array(analyser.fftSize / 2)
    detector.minVolumeDecibels = -10

    const samples = []
    processor.port.onmessage = e => {
      const frequency = new Uint8Array(analyser.frequencyBinCount)
      analyser.getByteFrequencyData(frequency)

      const pitchy = detector.findPitch(frequency, this.audioContext.sampleRate)

      const current = average(frequency)
      samples.push(current)
      if (samples.length > 1000) samples.shift()

      volume.set(current)
      min.update(min => Math.min(min, current))
      max.update(max => Math.max(max * 0.99, current))
      threshold.set(this.threshold + average(samples))
      trigger.set(+(current > threshold.get()))
      pitch.set(pitchy[0])
    }

    source.connect(analyser)
    analyser.connect(processor)

    return { volume, min, max, threshold, trigger, pitch }
  }
}
