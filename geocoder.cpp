#include "header.h"
#include <cctype>
#include <algorithm>

#ifndef NO_CURL
#include <curl/curl.h>
#endif

using namespace std;

string toLowerCity(const string& s)
{
    string out;
    out.reserve(s.size());
    for (char c : s)
        out += static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return out;
}

static string trim(const string& s)
{
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' ||
        s[a] == '\r' || s[a] == '\n'))
        a++;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' ||
        s[b - 1] == '\r' || s[b - 1] == '\n'))
        b--;
    return s.substr(a, b - a);
}

string normalizeCityName(const string& raw)
{
    string s = trim(raw);
    if (s.empty())
        return s;

    for (char c : s)
    {
        unsigned char uc = static_cast<unsigned char>(c);
        if (!isalpha(uc) && c != ' ' && c != '-')
        {
            return "";
        }
    }

    string collapsed;
    collapsed.reserve(s.size());
    bool prevSpace = false;
    for (char c : s)
    {
        if (c == ' ' || c == '\t')
        {
            if (!prevSpace)
                collapsed += ' ';
            prevSpace = true;
        }
        else
        {
            collapsed += c;
            prevSpace = false;
        }
    }
    bool startOfWord = true;
    for (size_t i = 0; i < collapsed.size(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(collapsed[i]);
        if (c == ' ' || c == '-')
        {
            startOfWord = true;
        }
        else if (startOfWord)
        {
            collapsed[i] = static_cast<char>(toupper(c));
            startOfWord = false;
        }
        else
        {
            collapsed[i] = static_cast<char>(tolower(c));
        }
    }
    return collapsed;
}

bool findCityInCSV(const string& cityName, double& lat, double& lon)
{
    ifstream f("cities.csv");
    if (!f.is_open())
        return false;

    string target = toLowerCity(trim(cityName));
    string line;
    getline(f, line); // header

    while (getline(f, line))
    {
        if (line.empty())
            continue;
        stringstream ss(line);
        string name, latStr, lonStr;
        getline(ss, name, ',');
        getline(ss, latStr, ',');
        getline(ss, lonStr, ',');

        if (toLowerCity(trim(name)) == target)
        {
            try
            {
                lat = stod(latStr);
                lon = stod(lonStr);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
    }
    return false;
}

//  Internals for geocode()

#ifndef NO_CURL
namespace
{
    size_t writeCB(void* ptr, size_t size, size_t nmemb, void* userdata)
    {
        size_t total = size * nmemb;
        string* out = static_cast<string*>(userdata);
        out->append(static_cast<char*>(ptr), total);
        return total;
    }

    string urlEncode(const string& s)
    {
        string out;
        out.reserve(s.size() * 3);
        for (char c : s)
        {
            unsigned char uc = static_cast<unsigned char>(c);
            if (isalnum(uc) || uc == '-' || uc == '_' ||
                uc == '.' || uc == '~')
            {
                out += static_cast<char>(uc);
            }
            else if (uc == ' ')
            {
                out += "%20";
            }
            else
            {
                char buf[4];
                snprintf(buf, sizeof(buf), "%%%02X", uc);
                out += buf;
            }
        }
        return out;
    }

    bool extractStringField(const string& body,
        const string& key, string& outVal)
    {
        string needle = "\"" + key + "\"";
        size_t p = body.find(needle);
        if (p == string::npos)
            return false;

        p = body.find(':', p + needle.size());
        if (p == string::npos)
            return false;
        p++;

        while (p < body.size() &&
            (body[p] == ' ' || body[p] == '\t'))
            p++;

        if (p < body.size() && body[p] == '"')
        {
            p++;
            size_t q = body.find('"', p);
            if (q == string::npos)
                return false;
            outVal = body.substr(p, q - p);
            return true;
        }

        size_t q = p;
        while (q < body.size() &&
            (isdigit(static_cast<unsigned char>(body[q])) ||
                body[q] == '-' || body[q] == '.' ||
                body[q] == '+' || body[q] == 'e' ||
                body[q] == 'E'))
            q++;
        if (q == p)
            return false;
        outVal = body.substr(p, q - p);
        return true;
    }
}
#endif
bool geocode(const string& cityName, double& lat, double& lon)
{
#ifdef NO_CURL
    (void)cityName;
    (void)lat;
    (void)lon;
    return false;
#else
    string clean = trim(cityName);
    if (clean.empty())
        return false;

    CURL* curl = curl_easy_init();
    if (!curl)
        return false;

    string query = clean + ", Pakistan";
    string url = "https://nominatim.openstreetmap.org/search?q=" + urlEncode(query) + "&format=json&limit=1";

    string body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCB);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
        "TravelTourSystem/1.0 (student-project)");

    CURLcode res = curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || httpCode != 200 || body.empty())
        return false;

    string trimmedBody = trim(body);
    if (trimmedBody == "[]")
        return false;

    string latStr, lonStr;
    if (!extractStringField(body, "lat", latStr))
        return false;
    if (!extractStringField(body, "lon", lonStr))
        return false;

    try
    {
        lat = stod(latStr);
        lon = stod(lonStr);
    }
    catch (...)
    {
        return false;
    }
    return true;
#endif
}