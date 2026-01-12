#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <esp_gattc_api.h>

#include "debug/debug.h"
#include "global/global.h"
#include "controller/controller.h"

static BLEClient *bleClient = nullptr;
static BLERemoteCharacteristic *bleRxCharacteristic = nullptr;
static BLERemoteCharacteristic *bleTxCharacteristic = nullptr;
static bool stopConnectFlag = false;
static BLEStatusType bleStatus = BLE_DISCONNECTED;

static BLEScan *scan;
static uint32_t scanStartMs = 0;

QueueHandle_t bleRxQueue = nullptr;
QueueHandle_t bleTxQueue = nullptr;
QueueHandle_t bleCmdQueue = nullptr;
QueueHandle_t bleScanEventQueue = nullptr;

// ----------------------------------------------------------------
//        BLE RX CALLBACK
// ----------------------------------------------------------------

void BLERxNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  BLERxEvent ev = {};
  ev.len = min(length, sizeof(ev.json));
  memcpy(ev.json, pData, ev.len);
  xQueueSendFromISR(bleRxQueue, &ev, nullptr);
}

// ----------------------------------------------------------------
//        BLE SCAN CALLBACK
// ----------------------------------------------------------------

class BLEScanCallbacks : public BLEAdvertisedDeviceCallbacks
{
public:
  void onResult(BLEAdvertisedDevice device) override
  {

    BLEScanEvent ev = {};
    ev.type = BLE_SCAN_RESULT;

    if (device.haveName())
    {
      strncpy(ev.result.name, device.getName().c_str(), DISP_MAX_MENU_TEXT - 1);
    }
    else
    {
      strncpy(ev.result.name, device.getAddress().toString().c_str(),
              DISP_MAX_MENU_TEXT - 1);
    }

    memcpy(ev.result.address, device.getAddress().getNative(), 6);

    xQueueSend(bleScanEventQueue, &ev, 0);
  }
};

static void startBLEScanInternal()
{

  BLEScanEvent ev = {};
  ev.type = BLE_SCAN_STARTED;
  xQueueSend(bleScanEventQueue, &ev, 0);

  debugLog(DBG_INFO, "BLE", "SCAN -> started");

  scan = BLEDevice::getScan();
  scan->clearResults();
  scan->setAdvertisedDeviceCallbacks(new BLEScanCallbacks());
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(99);

  scan->start(BLE_SCAN_DURATION_SEC, true);
  scanStartMs = millis();
}

static void stopBLEScanInternal()
{

  if (scan)
  {
    scan->stop();
  }

  stopConnectFlag = false;
  debugLog(DBG_INFO, "BLE", "SCAN -> stopped");

  BLEScanEvent ev = {};
  ev.type = BLE_SCAN_FINISHED;
  xQueueSend(bleScanEventQueue, &ev, 0);
}

// ----------------------------------------------------------------
//        BLETask
// ----------------------------------------------------------------

void BLETask(void *pv)
{

  BLEDevice::init("CNC-Pendant");
  BLEDevice::setMTU(BLE_MTU);
  bleClient = BLEDevice::createClient();
  debugLog(DBG_INFO, "BLE", "Client created");

  for (;;)
  {

    BLECmd cmd;

    if (xQueueReceive(bleCmdQueue, &cmd, 0))
    {

      if ((cmd.type == BLE_SCAN_START) && (bleStatus != BLE_SCANNING))
      {
        debugLog(DBG_INFO, "BLE", "SCAN -> requested");
        bleStatus = BLE_SCANNING;
        stopConnectFlag = true;
        startBLEScanInternal();
      }

      if ((cmd.type == BLE_SCAN_STOP) && (bleStatus == BLE_SCANNING))
      {
        debugLog(DBG_INFO, "BLE", "Scan stopped");
        stopBLEScanInternal();
        bleStatus = BLE_DISCONNECTED;
      }

      if ((cmd.type == BLE_DISCONNECT))
      {
        if (bleClient && bleClient->isConnected())
        {
          debugLog(DBG_INFO, "BLE", "Disconnect");
          bleClient->disconnect();
        }
        break;
      }
    }


    switch (bleStatus)
    {

    case BLE_DISCONNECTED:

      if (stopConnectFlag)
      {
        break;
      }

      debugLog(DBG_INFO, "BLE", "Connecting...");

      if (bleClient->connect(config.BluetoothHost))
      {

        esp_ble_gattc_send_mtu_req(
            bleClient->getGattcIf(),
            bleClient->getConnId());

        vTaskDelay(pdMS_TO_TICKS(300));

        sendGCode("msg", "[PENDANT] Connected", "Connected");
        debugLog(DBG_INFO, "BLE", "Connected");

        BLERemoteService *service = bleClient->getService(BLE_SERVICE_UUID);

        bleStatus = BLE_CONNECTED;

        if (service)
        {
          bleRxCharacteristic = service->getCharacteristic(BLE_RX_UUID);
          bleTxCharacteristic = service->getCharacteristic(BLE_TX_UUID);

          if (bleRxCharacteristic)
          {
            bleRxCharacteristic->registerForNotify(BLERxNotifyCallback);
            debugLog(DBG_INFO, "BLE", "Notify Callback registered");
          }
        }
        else
        {
          debugLog(DBG_ERROR, "BLE", "Service not found");
          bleClient->disconnect();
        }
      }
      else
      {
        debugLog(DBG_WARN, "BLE", "Connecting failed");
      }
      break;

    case BLE_SCANNING:
      if (millis() - scanStartMs >= BLE_SCAN_DURATION)
      {
        bleStatus = BLE_DISCONNECTED;
        stopBLEScanInternal();
        debugLog(DBG_INFO, "BLE", "SCAN -> finished");
      }
      break;

    case BLE_CONNECTED:

      // ---------------------------
      // Check if realy connected
      // ---------------------------
      if (!bleClient->isConnected())
      {
        bleStatus = BLE_DISCONNECTED;
        debugLog(DBG_WARN, "BLE", "Disconnected");
        break;
      }

      // ---------------------------
      // CNC Commands senden
      // ---------------------------

      BLETxEvent txEv = {};

      if (bleTxCharacteristic && xQueueReceive(bleTxQueue, &txEv, 0))
      {

        bleTxCharacteristic->writeValue((uint8_t *)txEv.json, txEv.len);
      }

      break;
    }

    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void BLEDisconnect()
{
}