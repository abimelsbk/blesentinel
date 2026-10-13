#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLESecurity.h>
#include "lab_config.h"

static BLECharacteristic* gConfigCharacteristic = nullptr;

class ConfigCallbacks final : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    const std::string value = characteristic->getValue();

    Serial.printf("[GATT] Config write observed (%u bytes)\n",
                  static_cast<unsigned>(value.length()));

    // Demonstration validation only. The classic Arduino BLE callback
    // runs after the write value has been received; restoring a value
    // here does NOT guarantee an ATT-level write-rejected response.
    if (value == "10" || value == "20" || value == "30") {
      Serial.println("[GATT] Config value is in the allowed test set");
    } else {
      Serial.println("[GATT] Invalid config value; restoring stored value to 10");
      characteristic->setValue("10");
    }
  }
};

static void configureSecurity() {
#if LAB_HARDENED
  BLESecurity* security = new BLESecurity();

  // Request bonding + MITM + LE Secure Connections.
  security->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);

  // This lab uses a fixed passkey for demonstration only.
  // ESP_IO_CAP_OUT is a display-only capability; the passkey is
  // printed to the USB serial monitor. This is not a production design.
  security->setCapability(ESP_IO_CAP_OUT);
  security->setStaticPIN(LAB_PASSKEY);

  Serial.println("[SECURITY] Hardened security requested");
  Serial.printf("[SECURITY] LAB ONLY passkey: %06lu\n", LAB_PASSKEY);
#else
  Serial.println("[SECURITY] Baseline mode: no protected attribute permissions configured");
#endif
}

void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("================================");
  Serial.println("       BLE Security Lab");
  Serial.println("================================");
  Serial.printf("Build mode: %s\n", LAB_HARDENED ? "HARDENED" : "BASELINE");

  BLEDevice::init(LAB_DEVICE_NAME);

#if LAB_HARDENED
  configureSecurity();
#endif

  BLEServer* server = BLEDevice::createServer();
  BLEService* service = server->createService(LAB_SERVICE_UUID);

  // Public characteristic intentionally remains readable in both modes.
  BLECharacteristic* publicInfo = service->createCharacteristic(
      LAB_PUBLIC_UUID,
      BLECharacteristic::PROPERTY_READ);
  publicInfo->setValue("BLE-LAB-01");

  BLECharacteristic* protectedInfo = service->createCharacteristic(
      LAB_PROTECTED_UUID,
      BLECharacteristic::PROPERTY_READ);
  protectedInfo->setValue("LAB-SECRET-42");

  gConfigCharacteristic = service->createCharacteristic(
      LAB_CONFIG_UUID,
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_WRITE |
      BLECharacteristic::PROPERTY_WRITE_NR);
  gConfigCharacteristic->setValue("10");
  gConfigCharacteristic->setCallbacks(new ConfigCallbacks());

  BLECharacteristic* status = service->createCharacteristic(
      LAB_STATUS_UUID,
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_NOTIFY);
  status->setValue("IDLE");

#if LAB_HARDENED
  // These permissions request an encrypted link with MITM protection.
  // Validate behavior on the exact core/stack/phone combination used.
  protectedInfo->setAccessPermissions(ESP_GATT_PERM_READ_ENC_MITM);
  gConfigCharacteristic->setAccessPermissions(
      ESP_GATT_PERM_READ_ENC_MITM |
      ESP_GATT_PERM_WRITE_ENC_MITM);
#endif

  service->start();

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(LAB_SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Advertising started");
  Serial.printf("[BLE] Device name: %s\n", LAB_DEVICE_NAME);
  Serial.printf("[BLE] Service UUID: %s\n", LAB_SERVICE_UUID);
  Serial.printf("[BLE] Public UUID: %s\n", LAB_PUBLIC_UUID);
  Serial.printf("[BLE] Protected UUID: %s\n", LAB_PROTECTED_UUID);
  Serial.printf("[BLE] Config UUID: %s\n", LAB_CONFIG_UUID);
  Serial.printf("[BLE] Status UUID: %s\n", LAB_STATUS_UUID);
  Serial.println("[BLE] Use synthetic lab data only.");
}

void loop() {
  delay(1000);
}
