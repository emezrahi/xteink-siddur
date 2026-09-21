# X3 Siddur Project: Current Progress

Repository: `emezrahi/xteink-siddur`  
Branch: `master`  
Starting commit: `4233a3d feat: add ordinary weekday Shaharit baseline`

## Implemented

### Date and zmanim engine

Added:

- `Zmanim.h`
  - Sunrise calculation
  - Solar noon calculation
  - Sunset calculation
  - Latitude/longitude validation
  - Timezone offset handling
  - Sunset detection

- `ServiceSelector.h`
  - Selects Shaharit, Minha, or Arvit based on local time and solar boundaries
  - Fallback schedule if zmanim are unavailable

- `CivilCalendar.h`
  - Gregorian date increment/decrement helpers

### Prayer context and Hebrew-date rollover

Expanded `PrayerContext` to include:

- Rosh Hodesh
- Yom Tov
- Hol HaMoed
- Fast days
- Hanukkah
- Purim
- Tachanun decision
- Ya’aleh Veyavo
- Al Hanissim
- Mussaf
- Aneinu
- Sefirat HaOmer and Omer day
- Hallel type
- Morid HaTal / Mashiv HaRuach
- Barechenu / Barech Aleinu

`PrayerContextResolver` now:

- Accepts service, date, sunset state, and diaspora setting
- Resolves the Hebrew date after sunset
- Resolves a live `CivilDateTime` plus location configuration
- Automatically selects the current service

### Edot HaMizrach calendar rules

Added `LiturgicalRules.h` with independent C++ rules for:

- Tachanun omissions
- Rosh Hodesh
- Ya’aleh Veyavo
- Hallel, half and full
- Mussaf
- Hanukkah and Al Hanissim
- Purim and Al Hanissim
- Minor fast days and postponed fasts
- Aneinu
- Sefirat HaOmer
- Seasonal Amidah wording
- Israel and diaspora rain-request timing

### Composer

`Composer` now supports:

- Shaharit
- Minha
- Arvit
- Mussaf
- Conditional Amidah variants
- Hallel
- Tachanun omission
- Torah reading conditions
- Mussaf insertion
- Omer insertion
- Holiday and fast-day blocks

The Smart Siddur APK remains a behavioral reference only. No decompiled Android implementation code or proprietary assets were copied.

### Device integration

Added persisted Siddur location settings to `CrossPointSettings`:

- Latitude, stored as millionths of a degree
- Longitude, stored as millionths of a degree
- Diaspora/Israel setting

The Siddur activity now:

- Uses the configured timezone
- Calculates sunset
- Rolls the Hebrew date after sunset
- Displays the resolved Hebrew date and recommended service

The default location is Brooklyn.

## Tests added

Added:

- `ZmanimTest.cpp`
- `LiturgicalRulesTest.cpp`

Updated existing Siddur composer tests for the expanded block model.

## Verification

The complete host-side test suite passes:

- **209/209 tests passed**
- Includes the new zmanim, service-selection, sunset-rollover, holiday, Omer, seasonal Amidah, and rain-request tests
- `git diff --check` passes
- Clang formatting was run successfully

## Firmware build status

PlatformIO and the ESP32 toolchain were installed locally. The default firmware build progressed through SDK and library installation, but the build process was interrupted while downloading the JPEGDEC dependency because the execution environment hit its tool approval/usage limit.

There is currently no confirmed firmware compile result yet. Continue with:

```bash
cd /workspace/scratch/93b4d04ac28d/xteink-siddur

export PATH=/root/.local/bin:$PATH
export SSL_CERT_FILE=/usr/local/share/ca-certificates/nebula-dns.crt
export REQUESTS_CA_BUNDLE=/usr/local/share/ca-certificates/nebula-dns.crt

pio run -e default
```

Then build the full CI matrix:

```bash
pio run -e default
pio run -e sticky
pio run -e x4pro
pio run -e papermono
```

After firmware builds succeed, run:

```bash
cmake --build build/test
ctest --test-dir build/test --output-on-failure -j
pio check --fail-on-defect low --fail-on-defect medium --fail-on-defect high
```

## Important remaining work

1. Finish all firmware builds.
2. Fix any ESP32-specific compile errors.
3. Run cppcheck.
4. Verify that all new prayer block IDs have corresponding Edot content blocks before exposing them in the reader UI.
5. Add a user-facing location/timezone configuration screen instead of relying only on persisted settings fields.
6. Add complete Minha, Arvit, Hallel, Mussaf, fast-day, Omer, and holiday text content.
7. Verify behavior on the physical X3.
