# Lite removals

This fork removes features from upstream CrossInk. Files are deleted for real;
`git rerere` is on, so repeated upstream merge conflicts resolve themselves.

After each upstream merge, re-check the call sites listed under each tier, and
look for upstream code that references a deleted file.

Baseline (clean `pio run -e default`, merged `lite` dd4dfc88): flash 6,380,377 B (97.4%), RAM 74,668 B.
Measure with a clean build (`rm -rf .pio/build/default`); incremental builds drifted by ~6.5 KB.

## Tier 1: server sync (done)

After (clean build): flash 6,213,147 B (94.8%), RAM 74,500 B (-167,230 B flash).

Kept on purpose: Nearby position/stats sync and Nearby book transfer. Nearby
position sync still uses `lib/KOReaderSync` (`ProgressMapper`,
`KOReaderDocumentId`, `KOReaderProgress`, `KOReaderCredentialStore`), and local
plumbing that only existed for the server (`src/sync/*`, `lib/SyncLog`,
`ReadingSessionJournal`, `SyncPending` markers) is left in place: it is spread
through the readers, and the linker drops what nothing calls.

Deleted:
- `src/activities/reader/KOReaderSyncActivity.*`
- `src/activities/settings/KOReader{Auth,Settings}Activity.*`
- `src/activities/network/StatsUploadActivity.*`
- `src/network/{ReadingSyncUpload,StatsUploadClient,ClippingsUpload}.*`

Call sites cut (re-check after upstream merges):
- `main.cpp`: `startGlobalSyncProgress()` is a stub; `KOREADER_SYNC`/`KOREADER_AUTH` boot targets log and ignore.
- `EpubReaderActivity.{h,cpp}`: kept-position offer flow removed; `EpubReaderMenuAction::SYNC` is a no-op.
- `BookActions.{h,cpp}`, `FileBrowserActivity.cpp`, `LibraryActivity.cpp`: Sync Book and Upload Folder actions removed.
- `NetworkModeSelectionActivity.cpp`, `CrossPointWebServerActivity.cpp`: Sync Books / Reading Stats / Clippings modes removed.
- `BookStatsActivity.cpp`: `canSyncReadingStats()` always false.
- `SettingsList.{h,cpp}`, `SettingsActivity.cpp`: server rows removed; Document Matching kept for Nearby.
- `Activity.cpp`, `FrontlightPanelActivity.cpp`, `EpubReaderMenuModel.h`: Sync Book entry removed.

Also removed: the simulator smoke-test scenarios and scripts that only exercised
server sync (`run_{kosync_settings,reading_upload,stats_upload}_smoke_test.py`,
`src/simulator/{StatsUpload,ReadingUpload}SmokeTest.*`, the `--context-menu-sync-return`,
`--file-browser-sync-return` and `--sync-server-captures` flags), and
`test/clipping_sync` (its `stubs/` stay: other tests include them).

Known leftovers:
- "Sync Book" stays a valid persisted shortcut ID but `QuickActions::isActionAvailable()` hides it from every picker.
- The simulator smoke test passes on the X3 profile after the v2.0.0 rebase: `scripts/run_simulator_smoke_test.py --env simulator-X3`.
- `lib/KOReaderSync/KOReaderSyncClient.cpp` (network client) is still compiled but unreferenced, except `readerId()` (used by `UsbProgressSync`).
- `docs/` still describes the removed sync features.

## Tier 2: OPDS and Calibre (done)

After (clean build): flash 6,127,491 B (93.5%), RAM 74,300 B (-85,656 B vs Tier 1; -252,886 B vs baseline).

Deleted:
- `lib/OpdsParser`, `src/OpdsServerStore.*`, `src/util/Opds{DownloadFolder,FeedError}.h`
- `src/activities/browser/` (OPDS book browser), `src/activities/settings/Opds{ServerList,Settings}Activity.*`
- `src/activities/network/CalibreConnectActivity.*`
- tests: `test/opds_parser/*` (its `stubs/` stay, `http_downloader` uses them), `test/opds_server_store`, `test/scripts/test_opds_feed_recovery.py`, `scripts/test_opds_download_folders.py`, `src/simulator/OpdsCatalogSmokeTest.*`

Call sites cut (re-check after upstream merges):
- `ActivityManager.{h,cpp}`: `goToBrowser()` / `goToOpdsServer()` removed; `goToCalibreWireless()` is an inert stub (reader, power-button and chord shortcuts still call it).
- `main.cpp`: `OPDS` network boot target logs and is ignored.
- `NetworkModeSelectionActivity.cpp`, `CrossPointWebServerActivity.cpp`: OPDS and Calibre rows and handlers removed (the enum values stay).
- `SettingsList.{h,cpp}`, `SettingsActivity.cpp`: OPDS Servers row and the `opdsDownloadFolder` web setting removed. The `opds*` fields in `CrossPointSettings` stay so the persisted layout is unchanged.
- `QuickActions.h`: Calibre Wireless is hidden from pickers like Sync Book.
- `CrossPointWebServer.{h,cpp}`, `web/pages/settings.{html,js,css}`: `/api/opds*` endpoints and the OPDS settings UI removed.
- `test/http_downloader`: OPDS feed-failure assertions and the `OpdsParser` test removed; the generic download tests stay.
- `src/simulator/PluginSmokeTest.cpp`, `SimulatorSmokeTest.cpp`, `scripts/run_simulator_smoke_test.py` (`--opds-catalog`): OPDS scenarios removed.

## Tier 3: network OTA and font download (done)

NTP clock sync (`ClockSyncActivity`, Settings > Clock sync now) was kept on request.

After (clean build, ESP-IDF 5.5.5): flash 6,088,033 B (92.9%), RAM 74,244 B (-39,458 B flash vs Tier 2's 6,127,491 B; -292,344 B vs baseline).
The Tier 2 figure was measured before the `memcpy` boot-loop fix below, so the delta is approximate.

Deleted:
- `src/network/OtaUpdater.cpp` and `src/activities/settings/OtaUpdateActivity.*` (Settings > Check for updates)
- `src/activities/settings/FontDownloadActivity.*` (Settings > Download fonts)

Kept on purpose:
- `src/network/OtaUpdater.h` (declaration only): the external `crossink-simulator` library has a `simulator_ota.cpp` that defines `OtaUpdater` and includes this header, so deleting it breaks the simulator build. Nothing on the device includes it.
- `FirmwareFlasher`, `FirmwareBoardTag`, `OtaBootSwitch` and `SdFirmwareUpdateActivity` (Settings > SD firmware update, also the boot-time recovery mode).
- `ClockSyncActivity`, `TrustedTime`, `HalClock`, manual clock offset.
- `HttpDownloader`, `AssetDownloadActivity` (language and hyphenation packs), `PluginHubInstallActivity`, `CatalogActivity`: still used.
- `NetworkBootTarget::OTA` / `MANAGE_FONTS` enum values (RTC-validated persisted values).

Call sites cut (re-check after upstream merges):
- `main.cpp`: `OTA` and `MANAGE_FONTS` network boot targets log and are ignored; `silentRestartToManageFonts()` removed (also from `SilentRestart.h`).
- `SettingsList.h`, `SettingsActivity.cpp`: the Check for updates and Download fonts rows removed; `CheckForUpdates` / `DownloadFonts` actions are no-op cases.
- `platformio.ini`: simulator `build_src_filter` no longer lists `OtaUpdater.cpp`.
- Tests: `test/ota_staging_guard` now checks only `FirmwareFlasher`; `scripts/test_build_identity.py` lost the font-manifest URL test; `test/scripts/test_transfer_reliability.py` lost the font-download retry test; `SimulatorSmokeTest.cpp` lost the OTA and font-download screen captures.

Known leftovers:
- `FONT_MANIFEST_URL` / `FONTS_MANIFEST_VERSION` macros lived in the deleted `FontDownloadActivity.h`; `lib/EpdFont/SdCardFont.h`, `lib/EpdFont/scripts/cpfont_version.py` and `.github/workflows/release-fonts.yml` still mention them in comments or for upstream's font release pipeline.
- Fonts now come only from the SD card (web server upload).
- The Reader > Download Font Size Range setting (`sdFontSizeRange`) was removed from `SettingsList.{h,cpp}` since it only chose which sizes to download. The `CrossPointSettings::sdFontSizeRange` field stays for the persisted layout; `SdCardFontSystem::resolveLegacySizeStep` still reads it as a fallback (always the default, `TINY`).

## Upstream rebase: release/v2.0.0

`lite` was rebased onto `upstream/release/v2.0.0` (0b939f41). After the rebase, a
clean-ish `pio run -e default` gives flash 6,126,461 B (93.5%), RAM 74,532 B. That
includes upstream's 65 new commits, so it is not purely lite's savings.

Conflict decisions (re-check on the next upstream merge):
- `freeink-sdk`: took upstream's pin (9d5bba3e). Lite's older "latest reachable" pin (d2600e95) is its ancestor.
- Upstream edits to files lite deletes were dropped, not ported: KOReader sync changes, the OPDS hardening (`6943c6aa`), and the clipping-sync and stats-upload tests.
- `SilentRestart.h`, `SettingsList.h`: kept upstream's new `bool` return types and new settings (EEGO two-tone, Ruled Lines); only the removed rows and `silentRestartToManageFonts()` were cut.

### USB progress sync (stubbed)

Upstream's `UsbProgressSync` (`b3aad1a5`, sync through Inky over USB) relays to the
removed `ReadingSyncUpload`, `StatsUploadClient` and `ClippingsUpload` clients.
`src/network/UsbProgressSync.cpp` is now a stub: the interface stays so
`UsbSerialFileTransfer.cpp` merges cleanly, `handle()` answers `unsupported`,
`expireBookList()` does nothing, and `readerId()` still calls `KOReaderSyncClient`.
`UsbSerialFileTransfer.cpp` reports `progress_sync=0` in its STATUS line. Deleted:
`scripts/test_usb_{progress_framing,progress_sync,sync_relay}.py`. USB file transfer
itself is unchanged. If upstream changes `UsbProgressSync`'s interface, update the stub.

### Settings capacity

`BASE_SETTINGS_CAPACITY` in `SettingsList.h` is hard-coded and the smoke test checks
it for exact equality with the real list size. Lite is 10 lower than upstream
(`opdsDownloadFolder`, 8 KOReader fields and `sdFontSizeRange`): 103 / 102 instead of 113 / 112. Any
upstream change to that constant needs the same 10 subtracted.

## Build gotcha: boot loop with a `memcpy` cache error

Symptom: the device boot-loops right after flashing with
`Guru Meditation Error: Core 0 panic'ed (Cache error). Cached memory region accessed while ibus or cache is disabled`,
and the faulting PC (`MEPC`) is `memcpy` at a `0x42xxxxxx` (flash) address, reached from
`esp_flash_init_default_chip` / `read_id_core`.

Cause: the toolchain's flash-resident `memcpy` was linked instead of IDF's IRAM/ROM copy. It happened with stale,
git-ignored generated files in the repo root (`sdkconfig.default`, `sdkconfig.defaults`); the same source built fine
earlier on the same pin (ESP-IDF 5.5.5), and upstream's release builds are fine too.

Fix: delete `sdkconfig.default` and `sdkconfig.defaults`, delete `~/.platformio/packages/framework-arduinoespressif32-libs`
and `.pio/build/<env>`, then rebuild. A good build has `memcpy` in IRAM (`0x4038xxxx`, not `0x42xxxxxx`):
`riscv32-esp-elf-gcc-nm -S .pio/build/default/firmware.elf | grep -w memcpy`.

## Planned

- Later: clippings.
