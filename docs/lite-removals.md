# Lite removals

This fork removes features from upstream CrossInk. Files are deleted for real;
`git rerere` is on, so repeated upstream merge conflicts resolve themselves.

After each upstream merge, re-check the call sites listed under each tier, and
look for upstream code that references a deleted file.

Baseline (clean `pio run -e default`, merged `lite` dd4dfc88): flash 6,380,377 B (97.4%), RAM 74,668 B.
Measure with a clean build (`rm -rf .pio/build/default`); incremental builds drifted by ~6.5 KB.

## Tier 1: server sync (done, uncommitted)

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
- The simulator build could not be run on the maintainer's machine (Apple clang 12.5 lacks C++20 `from_chars`); the smoke test was only compile-checked. Run `scripts/run_simulator_smoke_test.py` on a modern toolchain.
- `lib/KOReaderSync/KOReaderSyncClient.cpp` (network client) is still compiled but unreferenced, except `readerId()`.
- `docs/` still describes the removed sync features.

## Planned

- Tier 2: OPDS, Calibre connect.
- Tier 3: OTA, NTP clock sync, font download (check sharing with `HttpDownloader` first).
- Later: clippings.
