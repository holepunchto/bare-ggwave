# bare-ggwave

> 🤖 **Vibe coded experiment.** Built quickly with an AI assistant to explore an idea. Expect rough edges and breaking changes, it is not ready for production use.

Data over sound for Bare. Native bindings for [ggwave](https://github.com/ggerganov/ggwave), including custom protocols, for example an inaudible band.

```
npm i bare-ggwave
```

## Usage

```js
const GGWave = require('bare-ggwave')

const tx = new GGWave({ protocol: GGWave.protocols.AUDIBLE_FASTEST, payloadLength: 16 })
const samples = tx.encode(payload) // Float32Array, mono, 48 kHz

const rx = new GGWave({ protocol: GGWave.protocols.AUDIBLE_FASTEST, payloadLength: 16 })
for (let i = 0; i < mic.length; i += rx.samplesPerFrame) {
  const received = rx.decode(mic.subarray(i, i + rx.samplesPerFrame))
  if (received) console.log('got', received)
}
```

Define your own band before creating instances that use it:

```js
// 18.75 - 21.7 kHz, inaudible to most people
GGWave.define(GGWave.protocols.CUSTOM_0, { freqStart: 400, framesPerTx: 3, bytesPerTx: 2 })
```

## API

#### `const ggwave = new GGWave([options])`

```js
{
  sampleRate: 48000,
  payloadLength: -1, // -1 for variable length, or 1 - 64 for fixed length frames without markers (faster)
  protocol: GGWave.protocols.AUDIBLE_FASTEST, // the protocol to send and receive
  volume: 50
}
```

#### `GGWave.protocols`

Protocol ids: `AUDIBLE_*`, `ULTRASOUND_*`, `DT_*`, `MT_*` (`NORMAL`, `FAST`, `FASTEST`) and `CUSTOM_0` - `CUSTOM_9`.

#### `GGWave.define(id, { freqStart, framesPerTx, bytesPerTx, extra = 1 })`

Define or redefine a protocol for instances created after. The band starts at `freqStart * sampleRate / 1024` Hz. Each symbol lasts `framesPerTx` frames of 1024 samples and carries `bytesPerTx` bytes. A band is about `bytesPerTx * 1.4` kHz wide.

#### `const samples = ggwave.encode(payload)`

Encode a payload to float32 mono samples.

#### `const payload = ggwave.decode(samples)`

Feed float32 samples, a whole number of frames of `ggwave.samplesPerFrame`. Returns the payload once one has been received, otherwise `null`.

#### `ggwave.receiving`

Whether a transmission is currently being received (variable length mode).

#### `ggwave.destroy()`

Free the native instance now. An instance that is never destroyed is freed when it is garbage collected.

## License

Apache-2.0. ggwave is MIT licensed, see its repository.
