/* global AudioWorkletProcessor, registerProcessor */

registerProcessor(
  'processor',
  class Processor extends AudioWorkletProcessor {
    process (inputs, outputs, parameters) {
      this.port.postMessage(inputs[0])
      return true
    }
  }
)
