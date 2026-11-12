# Lite removals

This fork removes features from upstream CrossInk. Files are deleted for real;
`git rerere` is on, so repeated upstream merge conflicts resolve themselves.

After each upstream merge, re-check the call sites listed under each tier, and
look for upstream code that references a deleted file.

Baseline (`pio run -e default`, merged `lite` dd4dfc88): flash 6,373,917 B (97.3%), RAM 74,668 B.

## Tier 1: server sync (done, uncommitted)

After: flash 6,206,639 B (94.7%), RAM 74,500 B (-167,278 B flash).

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

Known leftovers:
- `QuickActions.h` and power-button/chord actions still list "Sync Book"; it does nothing.
- `src/simulator/*SmokeTest*` and `test/clipping_sync` still reference deleted classes, so the simulator build and those tests are broken until updated.
- `lib/KOReaderSync/KOReaderSyncClient.cpp` (network client) is still compiled but unreferenced, except `readerId()`.

## Planned

- Tier 2: OPDS, Calibre connect.
- Tier 3: OTA, NTP clock sync, font download (check sharing with `HttpDownloader` first).
- Later: clippings.
