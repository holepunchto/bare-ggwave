const test = require('brittle')
const GGWave = require('..')

const { protocols } = GGWave

test('variable length round trip', (t) => {
  const payload = Uint8Array.from('hello bare-ggwave', (c) => c.charCodeAt(0))
  t.alike(roundtrip({ protocol: protocols.AUDIBLE_FASTEST }, payload), payload)
})

test('fixed length round trip', (t) => {
  const payload = new Uint8Array(16).map((_, i) => i * 13)
  t.alike(roundtrip({ protocol: protocols.AUDIBLE_FASTEST, payloadLength: 16 }, payload), payload)
})

test('fixed length is faster than variable', (t) => {
  const payload = new Uint8Array(16)
  const variable = new GGWave({ protocol: protocols.AUDIBLE_FASTEST })
  const fixed = new GGWave({ protocol: protocols.AUDIBLE_FASTEST, payloadLength: 16 })
  t.teardown(() => {
    variable.destroy()
    fixed.destroy()
  })

  t.ok(fixed.encode(payload).length < variable.encode(payload).length)
})

test('custom inaudible protocol round trip', (t) => {
  GGWave.define(protocols.CUSTOM_0, { freqStart: 400, framesPerTx: 3, bytesPerTx: 2 })
  const payload = new Uint8Array(16).map((_, i) => 255 - i)
  t.alike(roundtrip({ protocol: protocols.CUSTOM_0, payloadLength: 16 }, payload), payload)
})

test('only decodes its own protocol', (t) => {
  const payload = new Uint8Array(16).fill(7)
  const tx = new GGWave({ protocol: protocols.ULTRASOUND_FASTEST, payloadLength: 16 })
  const rx = new GGWave({ protocol: protocols.AUDIBLE_FASTEST, payloadLength: 16 })
  t.teardown(() => {
    tx.destroy()
    rx.destroy()
  })

  t.is(feed(rx, tx.encode(payload)), null)
})

test('silence decodes to nothing', (t) => {
  const rx = new GGWave()
  t.teardown(() => rx.destroy())

  t.is(feed(rx, new Float32Array(48000)), null)
  t.is(rx.receiving, false)
})

function roundtrip(opts, payload) {
  const tx = new GGWave(opts)
  const rx = new GGWave(opts)
  const out = feed(rx, tx.encode(payload))
  tx.destroy()
  rx.destroy()
  return out
}

function feed(rx, samples) {
  const padded = new Float32Array(Math.ceil(samples.length / 1024) * 1024 + 48000)
  padded.set(samples)

  let out = null
  for (let i = 0; i < padded.length; i += 1024) {
    out = rx.decode(padded.subarray(i, i + 1024)) || out
  }
  return out
}
