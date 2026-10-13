# BLE Protocol Security Testing Lab

A hands-on BLE security assessment lab for **Windows 11 + Android + ESP32 DevKit**. An M5StickC Plus 2 can be added as a second BLE endpoint. A typical ESP8266 D1 Mini does not have native BLE.

> **Authorization:** Use this lab only with devices you own or are explicitly authorized to assess. The baseline mode intentionally exposes synthetic test values. Do not put real credentials, personal data, or production secrets in this lab.

## Repository layout

```text
ble-protocol-security-lab/
├── README.md
├── firmware/
│   └── ble_security_lab/
│       ├── ble_security_lab.ino
│       └── lab_config.h
├── tools/
│   ├── hash_evidence.ps1
│   └── log_test.py
├── test-cases/
│   └── test-matrix.csv
├── reports/
│   └── BLE-Security-Assessment-template.md
├── captures/
│   └── .gitkeep
└── evidence/
    ├── screenshots/.gitkeep
    └── serial-logs/.gitkeep
```

## What this lab covers

- BLE advertising and service discovery
- Pairing and bonding observations
- GATT read/write permissions
- Security upgrade behavior
- Application-level input validation
- Notifications and reconnection behavior
- Evidence logging and baseline-versus-hardened comparison

It does **not** provide passive over-the-air capture by itself. Windows host-side capture can only observe Bluetooth activity available to the Windows Bluetooth host. It will not capture the phone-to-ESP32 exchange when Windows is not part of that connection.

## Hardware roles

| Hardware | Role |
|---|---|
| Windows 11 laptop | Arduino IDE, serial monitor, Wireshark, evidence/report work |
| Android phone | BLE central using nRF Connect for Mobile |
| ESP32 DevKit (classic ESP32) | Main BLE peripheral |
| M5StickC Plus 2 | Optional second BLE device after the first lab works |
| D1 Mini (usually ESP8266) | Optional Wi-Fi experiments; not a native BLE endpoint |

## 1. Install tools

- [Arduino IDE](https://www.arduino.cc/en/software)
- [nRF Connect for Mobile](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile)
- [Wireshark](https://www.wireshark.org/download.html)

In Arduino IDE, add this Boards Manager URL under **File → Preferences**:

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

Open **Tools → Board → Boards Manager**, install **esp32 by Espressif Systems**, then select **ESP32 Dev Module** and the correct COM port.

This sketch uses the classic ESP32 Arduino BLE API. If an installed core version changes or removes an API, check Espressif's BLE examples and the compiler error before adapting the code. Record the core version used for your results.

## 2. Flash the firmware

1. Open `firmware/ble_security_lab/ble_security_lab.ino` in Arduino IDE.
2. Keep `#define LAB_HARDENED 0` for the isolated baseline.
3. Select **ESP32 Dev Module** and the correct serial port.
4. Click **Verify**, then **Upload**.
5. Open **Tools → Serial Monitor** at `115200` baud.
6. Confirm the output says `BLE Security Lab` and `Advertising started`.

The sketch uses synthetic data only:
- Public value: `BLE-LAB-01`
- Protected test value: `LAB-SECRET-42`
- Configuration value: `10`

## 3. Discover from Android

1. Install/open nRF Connect for Mobile.
2. Enable Bluetooth and grant nearby-device permissions.
3. Scan for `ASBK-BLE-LAB`.
4. Connect and wait for service discovery.
5. Expand the custom service and inspect all characteristics.
6. Read `PublicInfo`, `ProtectedInfo`, and `ConfigValue`.
7. Write `20` to `ConfigValue`, then read it again.
8. Try `999` and record the actual write result and the ESP32 serial output.
9. Disconnect/reconnect and repeat relevant tests.

A GATT `READ` or `WRITE` property only describes a supported operation; it does not prove the operation is authorized.

## 4. Run the baseline

With `LAB_HARDENED 0`, the protected test value is deliberately readable and the configuration characteristic has no BLE security permission configured by this sketch. This is only for your isolated lab.

Record:
- Can the phone read `ProtectedInfo` before pairing?
- Can it write `ConfigValue` before pairing?
- What happens when writing an invalid value?
- Does the behavior change after reconnecting?
- What does the serial log report?

Do not report these intentional baseline behaviors as a vulnerability in an external product.

## 5. Run hardened mode

1. Disconnect the phone.
2. Change `#define LAB_HARDENED 0` to `#define LAB_HARDENED 1`.
3. Upload the firmware again.
4. Forget any saved pairing on the phone.
5. Clear the matching bond on the ESP32 if one exists, using a supported bond-management method.
6. Re-scan and connect.
7. Attempt to read the protected characteristic before satisfying the required security.
8. Follow any pairing prompt and repeat the protected read and configuration write.
9. Record the negotiated behavior, errors, serial output, and screenshots.

The hardened build requests bonding, Secure Connections, and MITM protection and applies encrypted/MITM GATT permissions to protected attributes. Actual behavior depends on the ESP32 BLE stack, core version, IO capabilities, and phone. **Verify the negotiated security and observed results; do not assume that a compile or pairing prompt proves the control works.**

The sample uses a static lab passkey displayed in the serial monitor. It is a fixed demonstration value, not a production pairing design. Never reuse it in a real device. If the phone and peripheral cannot negotiate the configured association method, document the result and consult Espressif's maintained BLE security examples.

## 6. Test matrix

Use `test-cases/test-matrix.csv`. Record actual results; use `PASS`, `FAIL`, or `INCONCLUSIVE` only after observing behavior.

Minimum tests:
- A1: initial pairing method
- A2: bonded reconnect
- A3: bond removal/re-pair
- A4: authentication/MITM requirement
- B1: public characteristic read
- B2: protected read before required security
- B3: protected write before required security
- B4: invalid configuration value
- B5: alternate write mode if exposed by the client/stack
- C1: authorization policy
- D1: advertising and scan-response exposure
- D2: readable values and notifications

## 7. Evidence

Save screenshots under `evidence/screenshots/`, serial logs under `evidence/serial-logs/`, and capture files under `captures/`.

Hash evidence with PowerShell:

```powershell
.\tools\hash_evidence.ps1
```

This creates `evidence\capture-hashes.csv` for files in the captures directory. Hashes help detect changes after collection; they do not prove the evidence is accurate by themselves.

### Windows Bluetooth capture limitation

Wireshark's `etwdump` interface can capture Bluetooth activity available to the Windows host on supported Windows/Wireshark versions. It does not act as a passive BLE radio sniffer. If the Android phone connects directly to the ESP32, a Windows host capture will not independently capture that radio exchange. Use nRF Connect results and serial logs for those tests, and document the limitation. A compatible dedicated BLE sniffer is needed for independent over-the-air capture.

Useful Wireshark filters when supported by the capture:

```text
btle
btatt
btsmp
```

Encrypted payloads may remain unreadable. Do not publish sensitive keys or personal data.

## 8. Log tests

The optional helper script appends a row to the test CSV:

```powershell
python .\tools\log_test.py `
  --id B2 `
  --result INCONCLUSIVE `
  --actual "Fill in observed behavior" `
  --evidence "evidence/screenshots/B2.png"
```

Use a real observed result and replace the example text. The helper does not perform BLE tests automatically.

## 9. Findings and report

Use `reports/BLE-Security-Assessment-template.md`. Each finding should include:
- Title, ID, severity and rationale
- Affected device/firmware
- Preconditions and reproducible steps
- Expected versus actual behavior
- Evidence filenames
- Security impact
- Remediation and retest result

Do not invent findings or assign severity without evidence. Just Works is not automatically a vulnerability; assess it against the device's threat model. Pairing, encryption, authentication, and authorization are distinct concepts.

## References

- [Espressif Arduino ESP32 BLE library](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE)
- [Espressif secure static passkey example](https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/Server_secure_static_passkey/Server_secure_static_passkey.ino)
- [Espressif secure authorization example](https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/Server_secure_authorization/Server_secure_authorization.ino)
- [Wireshark Bluetooth capture guide](https://wiki.wireshark.org/CaptureSetup/Bluetooth)
- [Bluetooth SIG specifications](https://www.bluetooth.com/specifications/specs/)
- [CVSS](https://www.first.org/cvss/)
