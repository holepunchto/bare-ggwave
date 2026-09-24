declare class GGWave {
  constructor(opts?: {
    sampleRate?: number
    payloadLength?: number
    protocol?: number
    volume?: number
  })

  static readonly protocols: Record<string, number>
  static define(
    id: number,
    opts: { freqStart: number; framesPerTx: number; bytesPerTx: number; extra?: number }
  ): void

  readonly sampleRate: number
  readonly payloadLength: number
  readonly protocol: number
  readonly samplesPerFrame: number
  readonly receiving: boolean

  encode(payload: Uint8Array): Float32Array
  decode(samples: Float32Array): Uint8Array | null
  destroy(): void
}

export = GGWave
