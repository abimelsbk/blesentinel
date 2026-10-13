# BLE Protocol Security Testing Lab

A reproducible, authorized Bluetooth Low Energy (BLE) security
assessment lab using Windows 11, an Android phone, an ESP32 DevKit, an
M5StickC Plus 2.

> **Scope and safety:** This repository is for defensive testing on
> devices you own or are explicitly authorized to assess. Use synthetic
> data only. Do not test nearby third-party devices. Do not enable
> sensitive Bluetooth key logging on a daily-use computer. This project
> does not claim full over-the-air capture without compatible radio
> hardware.

## Project goals

Assess and document:

-   BLE discovery and advertising data exposure.
-   Pairing methods, authentication requirements, and bonding behavior.
-   GATT service and characteristic access controls.
-   Authentication versus application-level authorization.
-   Read, write, write-without-response, notification, and reconnection
    behavior.
-   Differences between an intentionally weak lab baseline and a
    hardened configuration.
-   Evidence collection, reproducibility, limitations, and remediation.

## Lab Inventory

The BLE security testing lab uses the following hardware and software components to assess Bluetooth Low Energy (BLE) pairing, bonding, GATT access controls, authentication, and data exposure.

### Hardware Inventory

| Component | Role | Purpose |
|---|---|---|
| Windows 11 Laptop | Development and analysis system | Run Arduino IDE, Wireshark, manage firmware, collect logs, and document test results |
| Android Smartphone | BLE central / test client | Discover BLE devices, connect to peripherals, inspect GATT services, and test characteristic reads and writes using nRF Connect for Mobile |
| ESP32 DevKit (ESP-WROOM-32) | Primary BLE peripheral | Host a custom BLE service with public and protected characteristics for security testing |
| M5StickC Plus 2 | Secondary BLE test device | Serve as an additional BLE peripheral or central for extended testing, depending on the firmware and BLE stack |
| USB Data Cable | Device connectivity | Flash firmware to development boards and collect serial logs |

### Software Inventory

| Software | Purpose |
|---|---|
| Arduino IDE | Develop, compile, and flash ESP32 firmware |
| ESP32 Arduino Core | Provide ESP32 board support and BLE functionality |
| nRF Connect for Mobile | BLE discovery, GATT enumeration, pairing, and characteristic testing |
| Wireshark | Analyze supported Bluetooth host-side captures and protocol events |
| Python 3 | Automate test-result logging and evidence processing |
| PowerShell | Manage the lab workspace and calculate SHA-256 hashes for evidence files |
| Git | Version control for firmware, test cases, documentation, and security reports |

### Optional Equipment

| Component | Purpose |
|---|---|
| Compatible BLE Sniffer | Capture over-the-air BLE traffic for deeper analysis of advertising, pairing, and connection events |

**Note:** A dedicated BLE sniffer is not required for the core lab activities. Without compatible radio-capture hardware, Windows host-side captures and application-level evidence cannot provide a complete passive over-the-air view of a phone-to-ESP32 connection.

## Architecture

``` text
Android phone (nRF Connect)
        | BLE connection / GATT operations
        v
ESP32 DevKit (test peripheral)
        |
        +-- PublicInfo
        +-- ProtectedInfo
        +-- ConfigValue
        +-- DeviceStatus

Windows 11 laptop
  +-- Arduino IDE / ESP32 core
  +-- USB serial logs
  +-- Wireshark (optional Windows Bluetooth HCI/ETW capture)
  +-- Evidence and report repository

M5StickC Plus 2: optional Wi-Fi/supporting experiments
```

## Security concepts

-   **Pairing:** Establishes keys and selects a security method. Pairing
    is not the same as authorization.
-   **Bonding:** Persists security information for future connections.
-   **Encryption:** Protects link traffic after encryption is
    established.
-   **MITM protection:** Depends on the negotiated association method
    and IO capabilities; it is not provided by Just Works.
-   **GATT properties:** Describe supported operations (read, write,
    notify, etc.).
-   **GATT permissions:** Determine the security required to perform
    operations.
-   **Application authorization:** Determines which authenticated client
    may perform a specific action. A bonded client is not automatically
    an administrator.
-   **Data exposure:** Advertising data and public GATT characteristics
    may disclose identifiers or sensitive values.

Do not report Just Works as a vulnerability automatically. Report it
when it conflicts with the documented threat model or security
requirement.

## Software

Install:

1.  [Arduino IDE](https://www.arduino.cc/en/software)
2.  [nRF Connect for
    Mobile](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile)
3.  [Wireshark](https://www.wireshark.org/download.html)
4.  ESP32 Arduino core from Espressif
5.  Optional: Git for Windows

References: - [ESP32 Arduino BLE library and security
examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE) -
[ESP32 Arduino BLE security
README](https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/README.md) -
[Wireshark Bluetooth capture
guide](https://wiki.wireshark.org/CaptureSetup/Bluetooth) - [Wireshark
`etwdump` manual](https://www.wireshark.org/docs/man-pages/etwdump) -
[Bluetooth
specifications](https://www.bluetooth.com/specifications/specs/) -
[CVSS](https://www.first.org/cvss/)

## 1. Create the workspace (Windows PowerShell)

Open PowerShell:

``` powershell
New-Item -ItemType Directory -Force -Path `
  "$HOME\ble-security-lab\firmware", `
  "$HOME\ble-security-lab\captures", `
  "$HOME\ble-security-lab\evidence\screenshots", `
  "$HOME\ble-security-lab\evidence\serial-logs", `
  "$HOME\ble-security-lab\reports", `
  "$HOME\ble-security-lab\test-cases"

Set-Location "$HOME\ble-security-lab"
Get-ChildItem
```

Optional installation commands (if `winget` is available):

``` powershell
winget install --id ArduinoSA.IDE.stable -e
winget install --id WiresharkFoundation.Wireshark -e
```

Package IDs can change; if a command fails, use the official download
links above.

## 2. Configure Arduino IDE

1.  Open Arduino IDE.

2.  Open **File → Preferences**.

3.  Add this Boards Manager URL:

    ``` text
    https://espressif.github.io/arduino-esp32/package_esp32_index.json
    ```

4.  Open **Tools → Board → Boards Manager**.

5.  Search for `esp32` and install **esp32 by Espressif Systems**.

6.  Select **Tools → Board → esp32 → ESP32 Dev Module** (menu names may
    differ).

7.  Connect the ESP32 DevKit over USB and select its COM port.

8.  Record the Arduino IDE and ESP32 core versions in the report.

## 3. Define the GATT service

Use a custom 128-bit service UUID and characteristic UUIDs. The sample
values below are synthetic.

  -----------------------------------------------------------------------
  Characteristic          Example value           Intended policy
  ----------------------- ----------------------- -----------------------
  `PublicInfo`            `BLE-LAB-01`            Public read

  `ProtectedInfo`         `LAB-SECRET-42`         Encrypted and
                                                  MITM-protected read in
                                                  hardened mode

  `ConfigValue`           `10`                    Read/write in baseline;
                                                  protected write in
                                                  hardened mode; validate
                                                  allowed values

  `DeviceStatus`          `IDLE`                  Read and notify
  -----------------------------------------------------------------------

Example UUIDs used in the companion sketch:

``` text
Service:       4fafc201-1fb5-459e-8fcc-c5c9c331914b
PublicInfo:    beb5483e-36e1-4688-b7f5-ea07361b26a8
ProtectedInfo: ff1d2614-e2d6-4c87-9154-6625d39ca7f8
ConfigValue:   d7c1aa01-36e1-4688-b7f5-ea07361b26a8
DeviceStatus:  a9c4d771-0f2e-4f16-8a32-4c6b1e5d9021
```

**Firmware:** Create `firmware/ble_security_lab/ble_security_lab.ino`
using the companion firmware maintained alongside this README, or adapt
Espressif's maintained BLE server examples. Verify APIs against the
installed core; BLE security APIs can differ between Bluedroid and
NimBLE. Do not assume compilation alone proves that security is
correctly enforced.

Official examples: - [Secure static passkey
example](https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/Server_secure_static_passkey/Server_secure_static_passkey.ino) -
[Secure authorization example and known
limitations](https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/Server_secure_authorization/Server_secure_authorization.ino)

## 4. Flash and verify baseline firmware

For the isolated baseline, use synthetic test data and no real secrets.

1.  Set the firmware's baseline switch to `0` (if using the supplied
    dual-mode sketch).
2.  Compile using **Verify**.
3.  Upload to the ESP32 DevKit.
4.  Open **Tools → Serial Monitor** and select `115200` baud.
5.  Confirm that the BLE server starts advertising and prints the
    expected service UUID.
6.  Save the output as `evidence/serial-logs/baseline.txt`.

If compilation fails, record the exact error and installed core version.
Do not proceed until the peripheral is advertising.

## 5. Discover and enumerate with Android

1.  Install and open nRF Connect for Mobile.
2.  Enable Bluetooth and grant nearby-device permissions.
3.  Start scanning and locate `ASBK-BLE-LAB`.
4.  Record advertised name, address type if shown, RSSI, advertised
    service UUIDs, and scan-response data.
5.  Connect and wait for service discovery.
6.  Expand the custom service and record the characteristic UUIDs and
    properties.
7.  Read `PublicInfo`, `ProtectedInfo`, and `ConfigValue`.
8.  Write an allowed test value such as `20` to `ConfigValue`, then read
    it again.
9.  Try an invalid synthetic value such as `999`; record the actual
    result and serial log.
10. Disconnect/reconnect and repeat relevant tests.

A characteristic's `READ` or `WRITE` property is not proof that access
is authorized. Properties describe supported operations; security
permissions and server-side authorization determine whether the
operation should be permitted.

## 6. Pairing and bonding test procedure

Use a clean bond state where possible. Remove the saved pairing on the
phone and clear the corresponding bond on the peripheral using the
stack's supported bond-management method. Avoid indiscriminately erasing
all nonvolatile storage.

### Pairing

-   Record whether pairing is initiated on connection or on a protected
    operation.
-   Record the negotiated association method where observable: Just
    Works, passkey entry, numeric comparison, or another supported
    method.
-   Determine whether MITM protection is required by the design and
    negotiated in practice.
-   Record whether the link becomes encrypted, using available logs and
    capture evidence.
-   Do not infer the negotiated method solely from a static passkey
    being configured.

### Bonding and reconnect

-   Pair and bond using the supported method.
-   Disconnect and reconnect.
-   Verify whether the bonded relationship is reused.
-   Re-test protected reads and writes.
-   Remove the bond on the phone and peripheral, then verify that fresh
    pairing is required where expected.
-   Reboot the peripheral and repeat the reconnection test.

### GATT access-control matrix

  -----------------------------------------------------------------------
  ID                      Test                    Expected hardened
                                                  behavior
  ----------------------- ----------------------- -----------------------
  A1                      Initial pairing         Security method matches
                                                  the documented threat
                                                  model

  A2                      Reconnect after bonding Bond is reused as
                                                  intended; protected
                                                  attributes remain
                                                  protected

  A3                      Bond removal            Fresh pairing is
                                                  required where expected

  A4                      MITM requirements       Negotiated method
                                                  provides required
                                                  authentication

  B1                      Read public             Allowed if
                          characteristic before   intentionally public
                          pairing                 

  B2                      Read protected          Denied or security
                          characteristic before   upgrade required
                          required security       

  B3                      Write protected         Denied or security
                          configuration before    upgrade required
                          required security       

  B4                      Write invalid           Rejected by application
                          configuration value     validation

  B5                      Alternate supported     No bypass of required
                          write modes             access controls

  B6                      Read protected          Allowed only when
                          characteristic after    required security is
                          pairing                 satisfied

  C1                      Authorization / role    Server enforces the
                          check                   intended role and
                                                  permissions

  C2                      Reconnect and repeat    Security policy remains
                          access                  enforced

  D1                      Advertising data review No unnecessary
                                                  sensitive data

  D2                      Notifications and GATT  No unintended
                          data review             disclosure
  -----------------------------------------------------------------------

For every test, record preconditions, expected behavior, actual
behavior, evidence, and a conclusion of **pass**, **fail**, or
**inconclusive**. An error response alone does not prove the correct
security control caused the failure.

## 7. Baseline versus hardened configuration

Maintain separate baseline and hardened firmware builds.

-   **Baseline:** intentionally exposes only synthetic lab test data to
    demonstrate what weak access controls look like.
-   **Hardened:** requests the security level required by the threat
    model and configures GATT permissions accordingly.

After changing firmware:

1.  Disconnect the phone.
2.  Flash the new build.
3.  Clear stale pairing state where needed.
4.  Re-scan and rediscover services.
5.  Run the same tests in the same order.
6.  Save new logs and screenshots.
7.  Compare actual outcomes, not just expected outcomes.

A bonded client is not automatically authorized for every operation. If
a characteristic needs role-based authorization, implement and test that
policy in the server/application, not only in the pairing configuration.

## 8. Windows 11 Bluetooth capture without a dedicated sniffer

Windows can expose host-side Bluetooth traffic through Wireshark's
`etwdump` interface, but this is not a passive radio sniffer.

1.  Install Wireshark **4.7.3 or later**.
2.  During installation, ensure `etwdump` is included.
3.  Run Wireshark as Administrator.
4.  Select `etwdump`.
5.  Configure the **Bluetooth Host Radio** provider.
6.  Start capture and perform a Bluetooth connection involving the
    Windows host itself.
7.  Stop and save the capture under `captures/`.

Example output names:

``` text
captures/windows-hci-baseline.pcapng
captures/windows-hci-hardened.pcapng
```

If Windows does not use its own Bluetooth adapter for the phone-to-ESP32
connection, the host-side capture will not record that exchange. Use nRF
Connect results and ESP32 serial logs for the phone-to-ESP32 test.
Document the limitation explicitly.

Useful Wireshark display filters, where supported by the capture:

``` text
btle
btatt
btsmp
```

Filter availability depends on the capture format and dissectors.
Encrypted payloads may remain unreadable. Do not enable sensitive key
logging on your daily-use laptop simply to make the trace easier to
inspect.

Reference: [Wireshark Bluetooth capture
guide](https://wiki.wireshark.org/CaptureSetup/Bluetooth).

## 9. Evidence management

Suggested repository layout:

``` text
ble-security-lab/
├── README.md
├── firmware/
│   └── ble_security_lab/
│       └── ble_security_lab.ino
├── captures/
│   ├── windows-hci-baseline.pcapng
│   └── windows-hci-hardened.pcapng
├── evidence/
│   ├── screenshots/
│   ├── serial-logs/
│   └── capture-hashes.csv
├── test-cases/
│   └── test-matrix.csv
└── reports/
    └── BLE-Security-Assessment.md
```

Hash capture files with PowerShell:

``` powershell
$root = "$HOME\ble-security-lab"

Get-ChildItem "$root\captures" -File |
    Get-FileHash -Algorithm SHA256 |
    Format-Table Path, Hash -AutoSize
```

Export hashes:

``` powershell
$root = "$HOME\ble-security-lab"

Get-ChildItem "$root\captures" -File |
    Get-FileHash -Algorithm SHA256 |
    Export-Csv "$root\evidence\capture-hashes.csv" -NoTypeInformation
```

Keep original evidence read-only where practical and sanitize copies
before publication. Never commit real credentials, pairing keys, IRKs,
personal data, or sensitive captures.

## 10. Findings and severity

Potential finding categories (examples only; do not claim these findings
without evidence):

-   Sensitive GATT value readable without required security.
-   Protected configuration can be modified by an unauthorized client.
-   Pairing method lacks required MITM protection.
-   Application authorization is missing or inconsistently enforced.
-   Sensitive identifiers or telemetry are exposed unnecessarily in
    advertising or GATT data.
-   Security policy is not preserved after reconnect or bond changes.

Each confirmed finding should include:

1.  ID and title.
2.  Severity and rationale.
3.  Affected firmware and component.
4.  Preconditions and reproducible steps.
5.  Expected and actual behavior.
6.  Evidence filename and packet/log references.
7.  Security impact.
8.  Remediation.
9.  Retest outcome.

Use [CVSS](https://www.first.org/cvss/) or another consistent
methodology where appropriate. Do not assign a severity without
considering exploitability, required proximity/access, affected data,
and realistic impact.

## 11. Security report template

Create `reports/BLE-Security-Assessment.md` with these sections:

``` markdown
# BLE Protocol Security Assessment

## Executive Summary
## Scope and Authorization
## Lab Architecture
## Device and Firmware Inventory
## Methodology
## Pairing and Bonding Results
## GATT Access-Control Results
## Authentication and Authorization Results
## Data Exposure Results
## Findings and Severity
## Baseline vs. Hardened Comparison
## Remediation and Retest
## Limitations
## Conclusion
## Appendix: Test Matrix and Evidence Inventory
```

## 12. Four-week work plan

### Week 1 --- Build

-   Install tools and verify USB serial.
-   Build and flash the BLE peripheral.
-   Discover the service and characteristics from the phone.
-   Record device, firmware, and library versions.

### Week 2 --- Test

-   Complete pairing and bonding tests.
-   Test GATT reads/writes and validation.
-   Review advertising and notifications.
-   Record expected and actual outcomes.

### Week 3 --- Harden and capture

-   Apply intended security requirements.
-   Repeat the exact baseline test cases.
-   Attempt Windows host-side Bluetooth capture where supported.
-   Document capture limitations.

### Week 4 --- Report

-   Verify and hash evidence.
-   Write findings with reproducible steps.
-   Compare baseline and hardened behavior.
-   Publish a sanitized report and repository.

## Completion criteria

-   [ ] Baseline firmware builds and advertises.
-   [ ] Phone discovers the service and all test characteristics.
-   [ ] Pairing/bonding behavior is documented.
-   [ ] GATT read/write behavior is recorded.
-   [ ] Hardened configuration is tested.
-   [ ] Baseline and hardened outcomes are compared.
-   [ ] Capture limitations are documented honestly.
-   [ ] Evidence is sanitized and hashed.
-   [ ] Findings are reproducible and the report is complete.

## Limitations

This lab is not a certification test, formal Bluetooth qualification, or
proof that a commercial product is secure. Without a compatible BLE
sniffer, it cannot independently capture arbitrary over-the-air traffic
between the phone and ESP32. Results apply only to the tested hardware,
firmware, library version, configuration, and test conditions.

## References

-   [Espressif Arduino ESP32 BLE
    library](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE)
-   [Espressif BLE security
    README](https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/README.md)
-   [Secure static passkey
    example](https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/Server_secure_static_passkey/Server_secure_static_passkey.ino)
-   [Secure authorization example and
    limitations](https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/Server_secure_authorization/Server_secure_authorization.ino)
-   [Wireshark Bluetooth capture
    guide](https://wiki.wireshark.org/CaptureSetup/Bluetooth)
-   [Wireshark `etwdump`
    manual](https://www.wireshark.org/docs/man-pages/etwdump)
-   [Bluetooth SIG
    specifications](https://www.bluetooth.com/specifications/specs/)
-   [CVSS](https://www.first.org/cvss/)
