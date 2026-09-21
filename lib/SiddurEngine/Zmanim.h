#pragma once

#include <cmath>

#include "CivilDateTime.h"

namespace SiddurEngine {

struct LocationConfig {
  double latitude = 40.6501;
  double longitude = -73.9496;
  int utcOffsetMinutes = -300;
  bool diaspora = true;
};

struct DailyZmanim {
  int sunriseMinutes = 0;
  int solarNoonMinutes = 0;
  int sunsetMinutes = 0;
  bool valid = false;
};

class Zmanim final {
 public:
  [[nodiscard]] static DailyZmanim calculate(const CivilDate date, const LocationConfig& location) {
    if (location.latitude < -90.0 || location.latitude > 90.0 || location.longitude < -180.0 ||
        location.longitude > 180.0 || location.utcOffsetMinutes < -720 || location.utcOffsetMinutes > 840) {
      return {};
    }

    const double julianDay = gregorianJulianDay(date);
    const double westLongitude = -location.longitude;
    const double cycle = std::round(julianDay - 2451545.0009 - westLongitude / 360.0);
    const double approximateNoon = 2451545.0009 + westLongitude / 360.0 + cycle;
    const double meanAnomaly = normalizeDegrees(357.5291 + 0.98560028 * (approximateNoon - 2451545.0));
    const double center = 1.9148 * sinDegrees(meanAnomaly) + 0.0200 * sinDegrees(2.0 * meanAnomaly) +
                          0.0003 * sinDegrees(3.0 * meanAnomaly);
    const double eclipticLongitude = normalizeDegrees(meanAnomaly + center + 180.0 + 102.9372);
    const double transit =
        approximateNoon + 0.0053 * sinDegrees(meanAnomaly) - 0.0069 * sinDegrees(2.0 * eclipticLongitude);
    const double declination = asinDegrees(sinDegrees(eclipticLongitude) * sinDegrees(23.4397));
    const double denominator = cosDegrees(location.latitude) * cosDegrees(declination);
    if (std::fabs(denominator) < 1e-12) return {};

    // Standard apparent sunrise/sunset altitude, including refraction and solar radius.
    const double cosHourAngle =
        (sinDegrees(-0.833) - sinDegrees(location.latitude) * sinDegrees(declination)) / denominator;
    if (cosHourAngle < -1.0 || cosHourAngle > 1.0) return {};

    const double hourAngleDays = acosDegrees(cosHourAngle) / 360.0;
    const double offsetDays = static_cast<double>(location.utcOffsetMinutes) / 1440.0;
    DailyZmanim result;
    result.sunriseMinutes = julianToLocalMinutes(transit - hourAngleDays + offsetDays);
    result.solarNoonMinutes = julianToLocalMinutes(transit + offsetDays);
    result.sunsetMinutes = julianToLocalMinutes(transit + hourAngleDays + offsetDays);
    result.valid = true;
    return result;
  }

  [[nodiscard]] static bool isAfterSunset(const CivilDateTime& local, const LocationConfig& location) {
    const DailyZmanim zmanim = calculate(local.date, location);
    return zmanim.valid && local.hour * 60 + local.minute >= zmanim.sunsetMinutes;
  }

 private:
  static constexpr double kPi = 3.14159265358979323846;

  [[nodiscard]] static double sinDegrees(const double degrees) { return std::sin(degrees * kPi / 180.0); }
  [[nodiscard]] static double cosDegrees(const double degrees) { return std::cos(degrees * kPi / 180.0); }
  [[nodiscard]] static double asinDegrees(const double value) { return std::asin(value) * 180.0 / kPi; }
  [[nodiscard]] static double acosDegrees(const double value) { return std::acos(value) * 180.0 / kPi; }

  [[nodiscard]] static double normalizeDegrees(double degrees) {
    degrees = std::fmod(degrees, 360.0);
    return degrees < 0.0 ? degrees + 360.0 : degrees;
  }

  [[nodiscard]] static int julianToLocalMinutes(const double julianDay) {
    double fraction = std::fmod(julianDay + 0.5, 1.0);
    if (fraction < 0.0) fraction += 1.0;
    int minutes = static_cast<int>(std::round(fraction * 1440.0));
    if (minutes >= 1440) minutes -= 1440;
    return minutes;
  }

  [[nodiscard]] static double gregorianJulianDay(const CivilDate date) {
    int year = date.year;
    int month = date.month;
    if (month <= 2) {
      --year;
      month += 12;
    }
    const int century = year / 100;
    const int correction = 2 - century + century / 4;
    return std::floor(365.25 * (year + 4716)) + std::floor(30.6001 * (month + 1)) + date.day + correction - 1524.5;
  }
};

}  // namespace SiddurEngine
