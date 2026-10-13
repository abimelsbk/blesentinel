#pragma once

// 0 = intentionally weak, isolated lab baseline using synthetic data
// 1 = request stronger BLE security and protect selected GATT attributes
#define LAB_HARDENED 0

#define LAB_DEVICE_NAME "ASBK-BLE-LAB"
#define LAB_PASSKEY 482731UL

#define LAB_SERVICE_UUID       "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define LAB_PUBLIC_UUID        "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define LAB_PROTECTED_UUID     "ff1d2614-e2d6-4c87-9154-6625d39ca7f8"
#define LAB_CONFIG_UUID        "d7c1aa01-36e1-4688-b7f5-ea07361b26a8"
#define LAB_STATUS_UUID        "a9c4d771-0f2e-4f16-8a32-4c6b1e5d9021"
