#include <assert.h>
#include <bare.h>
#include <ggwave/ggwave.h>
#include <js.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  GGWave *handle;
} bare_ggwave_t;

// the instance is freed by destroy, or when the handle is collected if destroy was never called
static void
bare_ggwave__finalize(js_env_t *env, void *data, void *finalize_hint) {
  bare_ggwave_t *ggwave = (bare_ggwave_t *) data;

  delete ggwave->handle;
  delete ggwave;
}

static bare_ggwave_t *
bare_ggwave__from(js_env_t *env, js_value_t *value) {
  int err;

  bare_ggwave_t *ggwave;
  err = js_get_value_external(env, value, (void **) &ggwave);
  assert(err == 0);

  return ggwave;
}

static js_value_t *
bare_ggwave_init(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 2;
  js_value_t *argv[2];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  double sample_rate;
  err = js_get_value_double(env, argv[0], &sample_rate);
  assert(err == 0);

  int32_t payload_length;
  err = js_get_value_int32(env, argv[1], &payload_length);
  assert(err == 0);

  GGWave::Parameters params = GGWave::getDefaultParameters();
  params.sampleRateInp = (float) sample_rate;
  params.sampleRateOut = (float) sample_rate;
  params.sampleRate = (float) sample_rate;
  params.payloadLength = payload_length;
  params.sampleFormatInp = GGWAVE_SAMPLE_FORMAT_F32;
  params.sampleFormatOut = GGWAVE_SAMPLE_FORMAT_F32;

  bare_ggwave_t *ggwave = new bare_ggwave_t;
  ggwave->handle = new GGWave(params);

  js_value_t *result;
  err = js_create_external(env, ggwave, bare_ggwave__finalize, NULL, &result);
  assert(err == 0);

  return result;
}

static js_value_t *
bare_ggwave_destroy(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 1;
  js_value_t *argv[1];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  bare_ggwave_t *ggwave = bare_ggwave__from(env, argv[0]);

  delete ggwave->handle;
  ggwave->handle = NULL;

  return NULL;
}

static js_value_t *
bare_ggwave_encode(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 4;
  js_value_t *argv[4];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  bare_ggwave_t *ggwave = bare_ggwave__from(env, argv[0]);

  void *data;
  size_t len;
  err = js_get_typedarray_info(env, argv[1], NULL, &data, &len, NULL, NULL);
  assert(err == 0);

  int32_t protocol;
  err = js_get_value_int32(env, argv[2], &protocol);
  assert(err == 0);

  int32_t volume;
  err = js_get_value_int32(env, argv[3], &volume);
  assert(err == 0);

  if (!ggwave->handle->init((int) len, (const char *) data, (GGWave::TxProtocolId) protocol, volume)) {
    err = js_throw_error(env, NULL, "Could not encode payload");
    assert(err == 0);
    return NULL;
  }

  uint32_t bytes = ggwave->handle->encode();

  js_value_t *result;

  void *out;
  err = js_create_arraybuffer(env, bytes, &out, &result);
  assert(err == 0);

  memcpy(out, ggwave->handle->txWaveform(), bytes);

  return result;
}

static js_value_t *
bare_ggwave_decode(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 2;
  js_value_t *argv[2];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  bare_ggwave_t *ggwave = bare_ggwave__from(env, argv[0]);

  void *data;
  size_t len;
  err = js_get_typedarray_info(env, argv[1], NULL, &data, &len, NULL, NULL);
  assert(err == 0);

  ggwave->handle->decode(data, (uint32_t) len);

  GGWave::TxRxData payload;
  int n = ggwave->handle->rxTakeData(payload);

  js_value_t *result;

  if (n <= 0) {
    err = js_get_null(env, &result);
    assert(err == 0);
    return result;
  }

  void *out;
  err = js_create_arraybuffer(env, n, &out, &result);
  assert(err == 0);

  memcpy(out, payload.data(), n);

  return result;
}

static js_value_t *
bare_ggwave_receiving(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 1;
  js_value_t *argv[1];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  bare_ggwave_t *ggwave = bare_ggwave__from(env, argv[0]);

  js_value_t *result;
  err = js_get_boolean(env, ggwave->handle->rxReceiving(), &result);
  assert(err == 0);

  return result;
}

// defines or redefines a protocol in the process wide rx and tx tables, affects instances created after
static js_value_t *
bare_ggwave_set_protocol(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 6;
  js_value_t *argv[6];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  int32_t values[5];
  for (int i = 0; i < 5; i++) {
    err = js_get_value_int32(env, argv[i], &values[i]);
    assert(err == 0);
  }

  bool enabled;
  err = js_get_value_bool(env, argv[5], &enabled);
  assert(err == 0);

  int id = values[0];

  GGWave::Protocols *tables[2] = {&GGWave::Protocols::tx(), &GGWave::Protocols::rx()};

  for (int i = 0; i < 2; i++) {
    GGWave::Protocol &p = (*tables[i])[id];
    if (p.name == NULL) p.name = "custom";
    p.freqStart = (int16_t) values[1];
    p.framesPerTx = (int8_t) values[2];
    p.bytesPerTx = (int8_t) values[3];
    p.extra = (int8_t) values[4];
    p.enabled = enabled;
  }

  return NULL;
}

static js_value_t *
bare_ggwave_toggle_rx(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 2;
  js_value_t *argv[2];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  int32_t id;
  err = js_get_value_int32(env, argv[0], &id);
  assert(err == 0);

  bool enabled;
  err = js_get_value_bool(env, argv[1], &enabled);
  assert(err == 0);

  GGWave::Protocols::rx()[id].enabled = enabled;

  return NULL;
}

static js_value_t *
bare_ggwave_exports(js_env_t *env, js_value_t *exports) {
  int err;

  ggwave_setLogFile(NULL);

#define V(name, fn) \
  { \
    js_value_t *val; \
    err = js_create_function(env, name, -1, fn, NULL, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, exports, name, val); \
    assert(err == 0); \
  }

  V("init", bare_ggwave_init)
  V("destroy", bare_ggwave_destroy)
  V("encode", bare_ggwave_encode)
  V("decode", bare_ggwave_decode)
  V("receiving", bare_ggwave_receiving)
  V("setProtocol", bare_ggwave_set_protocol)
  V("toggleRx", bare_ggwave_toggle_rx)
#undef V

  return exports;
}

BARE_MODULE(bare_ggwave, bare_ggwave_exports)
