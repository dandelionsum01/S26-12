//// ============================================================
////  Geocoder.cpp
////
////  Auto-fetches latitude/longitude for a city name using the
////  free OpenStreetMap Nominatim API. No API key required.
////
////  Public functions:
////      bool geocodeCity(const string& cityName,
////                       double& outLat, double& outLon);
////      bool findCachedCity(const string& cityName,
////                          double& outLat, double& outLon);
////      string normalizeCityName(const string& s);
////
////  Build:
////      Requires libcurl. Easiest install on Windows / Visual
////      Studio is via vcpkg:
////          vcpkg install curl:x64-windows
////          vcpkg integrate install
////
////      If libcurl is NOT available on the build machine, define
////      NO_CURL (or pass /D NO_CURL) and geocodeCity() will
////      simply return false. The cache lookup still works, so
////      the program keeps running -- the menu falls back to
////      manual lat/lon entry.
//// ============================================================
//#include "header.h"
//#include <algorithm>
//#include <cctype>
//
//#ifndef NO_CURL
//#include <curl/curl.h>
//#endif
//
//using namespace std;
//
//// ----------------------------------------------------------------
////  Helpers (file-local)
//// ----------------------------------------------------------------
//namespace
//{
//    // Strip CR / LF / spaces / tabs from the end of a string.
//    string trimEnd(string s)
//    {
//        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
//            s.back() == ' ' || s.back() == '\t'))
//            s.pop_back();
//        return s;
//    }
//
//    // Strip whitespace from the beginning of a string.
//    string trimStart(string s)
//    {
//        size_t i = 0;
//        while (i < s.size() && (s[i] == ' ' || s[i] == '\t'))
//            ++i;
//        return s.substr(i);
//    }
//
//    // Pull a JSON string field out by name. Returns "" if absent.
//    // We do this by hand instead of pulling in nlohmann/json so the
//    // project keeps compiling with zero extra dependencies.
//    //
//    // Looks for:  "key":"value"
//    string extractJsonString(const string& body, const string& key)
//    {
//        string needle = "\"" + key + "\"";
//        size_t k = body.find(needle);
//        if (k == string::npos) return "";
//
//        size_t colon = body.find(':', k + needle.size());
//        if (colon == string::npos) return "";
//
//        size_t firstQuote = body.find('"', colon + 1);
//        if (firstQuote == string::npos) return "";
//
//        size_t secondQuote = body.find('"', firstQuote + 1);
//        if (secondQuote == string::npos) return "";
//
//        return body.substr(firstQuote + 1, secondQuote - firstQuote - 1);
//    }
//
//    // URL-encode a city name (so spaces / & / etc. don't break the URL).
//    string urlEncode(const string& s)
//    {
//        string out;
//        const char hex[] = "0123456789ABCDEF";
//        for (unsigned char c : s)
//        {
//            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
//                out += static_cast<char>(c);
//            else if (c == ' ')
//                out += "%20";
//            else
//            {
//                out += '%';
//                out += hex[(c >> 4) & 0xF];
//                out += hex[c & 0xF];
//            }
//        }
//        return out;
//    }
//
//#ifndef NO_CURL
//    // libcurl write callback -- appends incoming bytes into a string.
//    size_t writeToString(void* contents, size_t size, size_t nmemb,
//        void* userPtr)
//    {
//        size_t total = size * nmemb;
//        string* out = static_cast<string*>(userPtr);
//        out->append(static_cast<char*>(contents), total);
//        return total;
//    }
//#endif
//}
//
//// ----------------------------------------------------------------
////  normalizeCityName
////
////  Used for *case-insensitive* compares only. We do NOT change
////  what gets written to cities.csv -- the user's chosen casing
////  is preserved on disk. We just compare in lowercase + trimmed
////  form so "Lahore", "LAHORE", and "  lahore" all match.
//// ----------------------------------------------------------------
//string normalizeCityName(const string& s)
//{
//    string out = trimStart(trimEnd(s));
//    transform(out.begin(), out.end(), out.begin(),
//        [](unsigned char c) { return static_cast<char>(tolower(c)); });
//    return out;
//}
//
//// ----------------------------------------------------------------
////  findCachedCity
////
////  Look the city up in cities.csv first (case-insensitive).
////  If found, fill outLat / outLon and return true.
//// ----------------------------------------------------------------
//bool findCachedCity(const string& cityName, double& outLat, double& outLon)
//{
//    ifstream f("cities.csv");
//    if (!f.is_open()) return false;
//
//    string target = normalizeCityName(cityName);
//
//    string line;
//    getline(f, line); // header
//
//    while (getline(f, line))
//    {
//        if (line.empty()) continue;
//
//        stringstream ss(line);
//        string name, latStr, lonStr;
//        getline(ss, name, ',');
//        getline(ss, latStr, ',');
//        getline(ss, lonStr, ',');
//
//        if (normalizeCityName(name) == target)
//        {
//            try
//            {
//                outLat = stod(latStr);
//                outLon = stod(lonStr);
//                return true;
//            }
//            catch (...)
//            {
//                return false; // malformed row, treat as miss
//            }
//        }
//    }
//    return false;
//}
//
//// ----------------------------------------------------------------
////  geocodeCity
////
////  Hits Nominatim (OpenStreetMap) and parses the first result.
////  Returns true on success.
////
////  Endpoint:
////    https://nominatim.openstreetmap.org/search
////        ?q=<city>,Pakistan&format=json&limit=1
//// ----------------------------------------------------------------
//bool geocodeCity(const string& cityName, double& outLat, double& outLon)
//{
//#ifdef NO_CURL
//    (void)cityName; (void)outLat; (void)outLon;
//    return false;
//#else
//    CURL* curl = curl_easy_init();
//    if (!curl) return false;
//
//    string response;
//    string url = "https://nominatim.openstreetmap.org/search?q="
//        + urlEncode(cityName) + ",Pakistan&format=json&limit=1";
//
//    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
//    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
//    // Nominatim REQUIRES a User-Agent. Without it, requests get blocked.
//    curl_easy_setopt(curl, CURLOPT_USERAGENT,
//        "TravelAndTourSystem/1.0 (student-project)");
//    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
//    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
//    // Skip cert verification on Windows where the CA bundle path is
//    // not reliably configured. For a student demo this is fine. For
//    // production code, ship a CA bundle and turn this back on.
//    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
//    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
//
//    CURLcode res = curl_easy_perform(curl);
//    curl_easy_cleanup(curl);
//
//    if (res != CURLE_OK || response.empty())
//        return false;
//
//    // Empty array returned by Nominatim -> city not found.
//    // The body looks like  []  or starts with whitespace then ].
//    {
//        string trimmed = response;
//        // strip leading whitespace
//        size_t i = 0;
//        while (i < trimmed.size() &&
//            (trimmed[i] == ' ' || trimmed[i] == '\n' ||
//                trimmed[i] == '\r' || trimmed[i] == '\t'))
//            ++i;
//        if (i + 1 < trimmed.size() && trimmed[i] == '[' && trimmed[i + 1] == ']')
//            return false;
//    }
//
//    string latStr = extractJsonString(response, "lat");
//    string lonStr = extractJsonString(response, "lon");
//
//    if (latStr.empty() || lonStr.empty())
//        return false;
//
//    try
//    {
//        outLat = stod(latStr);
//        outLon = stod(lonStr);
//        return true;
//    }
//    catch (...)
//    {
//        return false;
//    }
//#endif
//}