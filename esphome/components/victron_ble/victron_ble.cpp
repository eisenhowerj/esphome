#include "victron_ble.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32

#ifdef USE_BLE_TRACKER_PSA_AES
#include <psa/crypto.h>
#else
#include <mbedtls/aes.h>
#endif

namespace esphome {
namespace victron_ble {

static const char *const TAG = "victron_ble";

void VictronBle::dump_config() {
  ESP_LOGCONFIG(TAG, "Victron BLE");
  ESP_LOGCONFIG(TAG, "  Address: %012" PRIX64, this->address_);
}

void VictronBle::set_bindkey(const char *bindkey) {
  parse_hex(bindkey, this->bindkey_, sizeof(this->bindkey_));
}

bool VictronBle::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  if (device.address_uint64() != this->address_) {
    ESP_LOGVV(TAG, "parse_device(): unknown MAC address.");
    return false;
  }
  char addr_buf[MAC_ADDRESS_PRETTY_BUFFER_SIZE];
  const char *addr_str = device.address_str_to(addr_buf);
  ESP_LOGD(TAG, "parse_device(): MAC address %s found.", addr_str);

  for (auto &manufacturer_data : device.get_manufacturer_datas()) {
    // Manufacturer ID for Victron Energy BV is 0x02E1.
    // The UUID stores the ID little-endian so we check low byte (0xE1) first.
    if (!manufacturer_data.uuid.contains(0xE1, 0x02)) {
      ESP_LOGVV(TAG, "parse_device(): not a Victron manufacturer ID.");
      continue;
    }

    const auto &data = manufacturer_data.data;

    // Minimum: fixed header (8 bytes) plus at least 1 byte of encrypted payload.
    if (data.size() < sizeof(VictronManufacturerRecordHeader) + 1) {
      ESP_LOGVV(TAG, "parse_device(): manufacturer data too short (%zu bytes).", data.size());
      continue;
    }

    const auto *header = reinterpret_cast<const VictronManufacturerRecordHeader *>(data.data());

    if (header->record_type != VICTRON_MANUFACTURER_RECORD_TYPE) {
      ESP_LOGVV(TAG, "parse_device(): invalid manufacturer record type 0x%02X.", header->record_type);
      continue;
    }

    // A quick sanity check: byte 0 of the configured bindkey must match the key hint in the
    // advertisement.  This lets us reject packets not meant for our device before doing
    // full decryption.
    if (header->encryption_key_byte != this->bindkey_[0]) {
      ESP_LOGV(TAG, "parse_device(): encryption key byte mismatch (got 0x%02X).",
               header->encryption_key_byte);
      continue;
    }

    const VICTRON_BLE_RECORD_TYPE record_type =
        static_cast<VICTRON_BLE_RECORD_TYPE>(header->device_record_type);

    const uint8_t *ciphertext = data.data() + sizeof(VictronManufacturerRecordHeader);
    const uint8_t cipher_length =
        static_cast<uint8_t>(data.size() - sizeof(VictronManufacturerRecordHeader));

    // Build the 16-byte AES-CTR counter block: 16-bit nonce (little-endian) followed by zeros.
    uint8_t nonce_counter[16] = {0};
    nonce_counter[0] = header->nonce_lsb;
    nonce_counter[1] = header->nonce_msb;

    uint8_t plaintext[16] = {0};
    if (!this->decrypt_payload_(ciphertext, cipher_length, nonce_counter, plaintext)) {
      ESP_LOGW(TAG, "parse_device(): AES-CTR decryption failed.");
      continue;
    }

    ESP_LOGD(TAG, "parse_device(): decrypted %u byte(s) for record type 0x%02X.", cipher_length,
             static_cast<uint8_t>(record_type));

    this->on_victron_ble_record(record_type, plaintext, cipher_length);
    return true;
  }

  return false;
}

bool VictronBle::decrypt_payload_(const uint8_t *ciphertext, uint8_t cipher_length,
                                   const uint8_t *nonce_counter, uint8_t *plaintext) {
  if (cipher_length == 0 || cipher_length > 16) {
    ESP_LOGVV(TAG, "decrypt_payload_(): cipher_length %u is out of range.", cipher_length);
    return false;
  }

#ifdef USE_BLE_TRACKER_PSA_AES
  psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
  psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);
  psa_set_key_bits(&attributes, 128);
  psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DECRYPT);
  psa_set_key_algorithm(&attributes, PSA_ALG_CTR);

  mbedtls_svc_key_id_t key_id;
  if (psa_import_key(&attributes, this->bindkey_, sizeof(this->bindkey_), &key_id) != PSA_SUCCESS) {
    ESP_LOGVV(TAG, "decrypt_payload_(): psa_import_key() failed.");
    return false;
  }

  psa_cipher_operation_t operation = PSA_CIPHER_OPERATION_INIT;

  if (psa_cipher_decrypt_setup(&operation, key_id, PSA_ALG_CTR) != PSA_SUCCESS) {
    ESP_LOGVV(TAG, "decrypt_payload_(): psa_cipher_decrypt_setup() failed.");
    psa_destroy_key(key_id);
    return false;
  }

  if (psa_cipher_set_iv(&operation, nonce_counter, 16) != PSA_SUCCESS) {
    ESP_LOGVV(TAG, "decrypt_payload_(): psa_cipher_set_iv() failed.");
    psa_cipher_abort(&operation);
    psa_destroy_key(key_id);
    return false;
  }

  size_t output_length = 0;
  if (psa_cipher_update(&operation, ciphertext, cipher_length, plaintext, 16, &output_length) != PSA_SUCCESS) {
    ESP_LOGVV(TAG, "decrypt_payload_(): psa_cipher_update() failed.");
    psa_cipher_abort(&operation);
    psa_destroy_key(key_id);
    return false;
  }

  size_t final_output_length = 0;
  psa_status_t status =
      psa_cipher_finish(&operation, plaintext + output_length, 16 - output_length, &final_output_length);
  psa_destroy_key(key_id);
  return status == PSA_SUCCESS;
#else
  mbedtls_aes_context ctx;
  mbedtls_aes_init(&ctx);

  if (mbedtls_aes_setkey_enc(&ctx, this->bindkey_, 128) != 0) {
    ESP_LOGVV(TAG, "decrypt_payload_(): mbedtls_aes_setkey_enc() failed.");
    mbedtls_aes_free(&ctx);
    return false;
  }

  // mbedtls_aes_crypt_ctr may modify nonce_counter; work on a local copy.
  uint8_t nc[16];
  memcpy(nc, nonce_counter, sizeof(nc));
  size_t nc_off = 0;
  uint8_t stream_block[16] = {0};

  int ret = mbedtls_aes_crypt_ctr(&ctx, cipher_length, &nc_off, nc, stream_block, ciphertext, plaintext);
  mbedtls_aes_free(&ctx);

  if (ret != 0) {
    ESP_LOGVV(TAG, "decrypt_payload_(): mbedtls_aes_crypt_ctr() failed (%d).", ret);
    return false;
  }

  return true;
#endif
}

}  // namespace victron_ble
}  // namespace esphome

#endif  // USE_ESP32
