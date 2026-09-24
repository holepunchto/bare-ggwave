const binding = require('./binding')

const protocols = {
  AUDIBLE_NORMAL: 0,
  AUDIBLE_FAST: 1,
  AUDIBLE_FASTEST: 2,
  ULTRASOUND_NORMAL: 3,
  ULTRASOUND_FAST: 4,
  ULTRASOUND_FASTEST: 5,
  DT_NORMAL: 6,
  DT_FAST: 7,
  DT_FASTEST: 8,
  MT_NORMAL: 9,
  MT_FAST: 10,
  MT_FASTEST: 11,
  CUSTOM_0: 12,
  CUSTOM_1: 13,
  CUSTOM_2: 14,
  CUSTOM_3: 15,
  CUSTOM_4: 16,
  CUSTOM_5: 17,
  CUSTOM_6: 18,
  CUSTOM_7: 19,
  CUSTOM_8: 20,
  CUSTOM_9: 21
}

module.exports = class GGWave {
  constructor(opts = {}) {
    this.sampleRate = opts.sampleRate ?? 48000
    this.payloadLength = opts.payloadLength ?? -1
    this.protocol = opts.protocol ?? protocols.AUDIBLE_FASTEST
    this.volume = opts.volume ?? 50

    // the rx table is process wide and read when an instance is made, so only decode ours
    for (const id of Object.values(protocols)) binding.toggleRx(id, id === this.protocol)

    this._handle = binding.init(this.sampleRate, this.payloadLength)
  }

  static get protocols() {
    return protocols
  }

  // defines a protocol (usually a CUSTOM_ one) for instances created after, the band starts at
  // freqStart * sampleRate / 1024 Hz and each symbol lasts framesPerTx frames of 1024 samples
  static define(id, { freqStart, framesPerTx, bytesPerTx, extra = 1 }) {
    binding.setProtocol(id, freqStart, framesPerTx, bytesPerTx, extra, true)
  }

  get samplesPerFrame() {
    return 1024
  }

  get receiving() {
    return binding.receiving(this._handle)
  }

  // float32 mono samples at sampleRate
  encode(payload) {
    return new Float32Array(binding.encode(this._handle, payload, this.protocol, this.volume))
  }

  // feed float32 samples, a whole number of frames; returns a payload once one is complete
  decode(samples) {
    const bytes = new Uint8Array(samples.buffer, samples.byteOffset, samples.byteLength)
    const payload = binding.decode(this._handle, bytes)
    return payload === null ? null : new Uint8Array(payload)
  }

  destroy() {
    binding.destroy(this._handle)
    this._handle = null
  }
}
