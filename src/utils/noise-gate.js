// https://github.com/tennisonchan/noise-gate
/* global AudioParam, DynamicsCompressorNode, BiquadFilterNode */

const FILTER_PARAMS = ['type', 'frequency', 'gain', 'detune', 'Q']
const COMPRESSOR_PARAMS = ['threshold', 'knee', 'ratio', 'attack', 'release']
const DEFAULT_OPTIONS = {
  threshold: -50,
  knee: 40,
  ratio: 12,
  reduction: -20,
  attack: 0,
  release: 0.25,
  Q: 8.30,
  frequency: 355,
  gain: 3.0,
  type: 'bandpass'
}

export default class NoiseGate {
  constructor (audioCtx, options = {}) {
    options = Object.assign({}, DEFAULT_OPTIONS, options)

    const compressorPramas = this.selectParams(options, COMPRESSOR_PARAMS)
    const filterPramas = this.selectParams(options, FILTER_PARAMS)

    this.compressor = new DynamicsCompressorNode(audioCtx, compressorPramas)
    this.filter = new BiquadFilterNode(audioCtx, filterPramas)

    this.compressor.connect(this.filter)
    return this.filter
  }

  selectParams (object, filterArr) {
    return Object.keys(object).reduce(function (opt, p) {
      if (filterArr.includes(p)) {
        opt[p] = object[p]
      }
      return opt
    }, {})
  }

  setParams (node, audioParams) {
    for (const param in audioParams) {
      const value = audioParams[param]
      if (node[param] instanceof AudioParam) {
        node[param].value = value
      } else {
        node[param] = value
      }
    }
  }
}

// // const outputMix = audioContext.createGain()
// // const dryGain = audioContext.createGain()
// // const wetGain = audioContext.createGain()
// // const effectInput = audioContext.createGain()
// // audioInput.connect(dryGain)
// // audioInput.connect(effectInput)
// // dryGain.connect(outputMix)
// // wetGain.connect(outputMix)
// // outputMix.connect(audioContext.destination)
//
// // SEE https://cwilso.github.io/Audio-Input-Effects/
// export default class NoiseGate {
//   constructor (audioContext, wetGain, {
//     fl = 0.01, // [0.0, 0.1]
//     fq = 10.0 // [0.25; 20]
//   } = {}) {
//     const inputNode = audioContext.createGain()
//     const rectifier = audioContext.createWaveShaper()
//     const ngFollower = audioContext.createBiquadFilter()
//     ngFollower.type = 'lowpass'
//     ngFollower.frequency.value = fq

//     const curve = new Float32Array(65536)
//     for (let i = -32768; i < 32768; i++) {
//       curve[i + 32768] = ((i > 0) ? i : -i) / 32768
//     }

//     rectifier.curve = curve
//     rectifier.connect(ngFollower)

//     const ngGate = audioContext.createWaveShaper()
//     ngGate.curve = this.generateNoiseFloorCurve(fl)

//     ngFollower.connect(ngGate)

//     const gateGain = audioContext.createGain()
//     gateGain.gain.value = 0.0
//     ngGate.connect(gateGain.gain)

//     gateGain.connect(wetGain)

//     inputNode.connect(rectifier)
//     inputNode.connect(gateGain)
//     return inputNode
//   }

//   generateNoiseFloorCurve (floor) { // [0, 1]
//     const curve = new Float32Array(65536)
//     const mappedFloor = floor * 32768

//     for (let i = 0; i < 32768; i++) {
//       const value = (i < mappedFloor) ? 0 : 1
//       curve[32768 - i] = -value
//       curve[32768 + i] = value
//     }

//     curve[0] = curve[1] // fixing up the end.
//     return curve
//   }
// }
