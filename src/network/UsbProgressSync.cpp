#include "UsbProgressSync.h"

#include <Logging.h>

#include "KOReaderSyncClient.h"

// Lite fork: the USB progress-sync relay forwarded reading progress, stats and
// clippings to the removed server-sync clients. The interface is kept so
// UsbSerialFileTransfer merges cleanly with upstream; every request is refused.
namespace UsbProgressSync {

bool readerId(char* out, const size_t capacity) { return KOReaderSyncClient::readerId(out, capacity); }

void expireBookList(bool) {}

void handle(JsonVariantConst, JsonDocument& response, GfxRenderer&) {
  LOG_DBG("USBSYNC", "Progress sync refused: server sync is not available");
  response["error"] = "unsupported";
}

}  // namespace UsbProgressSync
