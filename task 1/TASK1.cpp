#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include <curl/curl.h> // For making HTTP requests

using json = nlohmann::json;
using namespace std;

// Helper function to handle HTTP responses
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Define the Location class
class Location {
public:
    string name;
    double latitude;
    double longitude;

    Location(string name, double lat, double lon) : name(name), latitude(lat), longitude(lon) {}
};

// Define the WeatherVariable class
class WeatherVariable {
public:
    string name;
    double value;

    WeatherVariable(const string& n, double v) : name(n), value(v) {}
};

// Define the WeatherForecastingSystem class
class WeatherForecastingSystem {
private:
    string apiKey;

public:
    WeatherForecastingSystem(const string& api_key) : apiKey(api_key) {}

    json fetchForecast(const Location& location) {
        CURL* curl;
        CURLcode res;
        string readBuffer;
        json data;

        curl = curl_easy_init();
        if (curl) {
            string url = "https://api.open-meteo.com/v1/forecast?latitude=" + to_string(location.latitude) + "&longitude=" + to_string(location.longitude) + "&hourly=temperature_2m,precipitation";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            }
            curl_easy_cleanup(curl);

            // Parse the response
            data = json::parse(readBuffer);
        }
        return data;
    }

    void displayForecast(const json& forecastData) const {
        cout << "Temperature: ";
        for (const auto& temp : forecastData["hourly"]["temperature_2m"]) {
            cout << temp << " ";
        }
        cout << endl;

        cout << "Precipitation: ";
        for (const auto& precip : forecastData["hourly"]["precipitation"]) {
            cout << precip << " ";
        }
        cout << endl;
    }
};

// Define the HistoricalWeatherSystem class
class HistoricalWeatherSystem {
private:
    string apiKey;

public:
    HistoricalWeatherSystem(const string& api_key) : apiKey(api_key) {}

    json fetchHistoricalData(const Location& location, const string& startDate, const string& endDate) {
        CURL* curl;
        CURLcode res;
        string readBuffer;
        json data;

        curl = curl_easy_init();
        if (curl) {
            string url = "https://api.open-meteo.com/v1/forecast?latitude=" + to_string(location.latitude) + "&longitude=" + to_string(location.longitude) + "&daily=temperature_2m_max,temperature_2m_min&start_date=" + startDate + "&end_date=" + endDate;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            }
            curl_easy_cleanup(curl);

            // Parse the response
            data = json::parse(readBuffer);
        }
        return data;
    }

    void displayHistoricalData(const json& historicalData) const {
        cout << "Historical Temperature Data: ";
        for (const auto& temp : historicalData["daily"]["temperature_2m_max"]) {
            cout << temp << " ";
        }
        cout << endl;
    }
};

// Define the AirQualityForecastingSystem class
class AirQualityForecastingSystem {
private:
    string apiKey;

public:
    AirQualityForecastingSystem(const string& api_key) : apiKey(api_key) {}

    json fetchAirQuality(const Location& location) {
        CURL* curl;
        CURLcode res;
        string readBuffer;
        json data;

        curl = curl_easy_init();
        if (curl) {
            string url = "https://api.waqi.info/feed/geo:" + to_string(location.latitude) + ";" + to_string(location.longitude) + "/?token=" + apiKey;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            }
            curl_easy_cleanup(curl);

            // Parse the response
            data = json::parse(readBuffer);
        }
        return data;
    }

    void displayAirQuality(const json& airQualityData) const {
        cout << "Air Quality Index: " << airQualityData["data"]["aqi"] << endl;
    }
};

// Define data export functions
void exportToCSV(const Location& location, const vector<WeatherVariable>& weatherVariables, const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        file << "Location: " << location.name << ", Latitude: " << location.latitude << ", Longitude: " << location.longitude << endl;
        file << "Weather Variable,Value" << endl;
        for (const auto& var : weatherVariables) {
            file << var.name << "," << var.value << endl;
        }
        file.close();
    }
    else {
        cout << "Failed to open file for writing." << endl;
    }
}

void exportToJSON(const Location& location, const vector<WeatherVariable>& weatherVariables, const string& filename) {
    json j;
    j["Location"]["Name"] = location.name;
    j["Location"]["Latitude"] = location.latitude;
    j["Location"]["Longitude"] = location.longitude;

    for (const auto& var : weatherVariables) {
        json varJson;
        varJson["name"] = var.name;
        varJson["value"] = var.value;
        j["WeatherData"].push_back(varJson);
    }

    ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
    }
    else {
        cout << "Failed to open file for writing." << endl;
    }
}

void exportToTXT(const Location& location, const vector<WeatherVariable>& weatherVariables, const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        file << "Location Information:\n";
        file << "  Name: " << location.name << "\n";
        file << "  Latitude: " << location.latitude << "\n";
        file << "  Longitude: " << location.longitude << "\n\n";
        file << "Weather Data:\n";
        for (const auto& var : weatherVariables) {
            file << "  " << var.name << ": " << var.value << "\n";
        }
        file.close();
    }
    else {
        cout << "Failed to open file for writing." << endl;
    }
}

// Define the main function with a console-based UI
int main() {
    LocationManager locationManager;
    WeatherForecastingSystem weatherSystem("YOUR_WEATHER_API_KEY");
    HistoricalWeatherSystem historicalSystem("YOUR_HISTORICAL_API_KEY");
    AirQualityForecastingSystem airQualitySystem("YOUR_AIR_QUALITY_API_KEY");

    while (true) {
        cout << "\n1. Add Location" << endl;
        cout << "2. Remove Location" << endl;
        cout << "3. List Locations" << endl;
        cout << "4. Fetch Weather Forecast" << endl;
        cout << "5. Fetch Historical Weather Data" << endl;
        cout << "6. Fetch Air Quality Data" << endl;
        cout << "7. Export Data" << endl;
        cout << "8. Exit" << endl;

        int choice;
        cin >> choice;

        switch (choice) {
        case 1: {
            string name;
            double latitude, longitude;
            cout << "Enter location name: ";
            cin >> name;
            cout << "Enter latitude: ";
            cin >> latitude;
            cout << "Enter longitude: ";
            cin >> longitude;
            Location location(name, latitude, longitude);
            locationManager.addLocation(location);
            break;
        }
        case 2: {
            string name;
            cout << "Enter location name to remove: ";
            cin >> name;
            locationManager.removeLocation(name);
            break;
        }
        case 3: {
            locationManager.listLocations();
            break;
        }
        case 4: {
            string name;
            cout << "Enter location name for forecast: ";
            cin >> name;
            Location* location = locationManager.findLocation(name);
            if (location) {
                json forecastData = weatherSystem.fetchForecast(*location);
                weatherSystem.displayForecast(forecastData);
            }
            else {
                cout << "Location not found." << endl;
            }
            break;
        }
        case 5: {
            string name, startDate, endDate;
            cout << "Enter location name for historical data: ";
            cin >> name;
            cout << "Enter start date (YYYY-MM-DD): ";
            cin >> startDate;
            cout << "Enter end date (YYYY-MM-DD): ";
            cin >> endDate;
            Location* location = locationManager.findLocation(name);
            if (location) {
                json historicalData = historicalSystem.fetchHistoricalData(*location, startDate, endDate);
                historicalSystem.displayHistoricalData(historicalData);
            }
            else {
                cout << "Location not found." << endl;
            }
            break;
        }
        case 6: {
            string name;
            cout << "Enter location name for air quality data: ";
            cin >> name;
            Location* location = locationManager.findLocation(name);
            if (location) {
                json airQualityData = airQualitySystem.fetchAirQuality(*location);
                airQualitySystem.displayAirQuality(airQualityData);
            }
            else {
                cout << "Location not found." << endl;
            }
            break;
        }
        case 7: {
            string name;
            double temperature, windSpeed;
            cout << "Enter location name for export: ";
            cin >> name;
            cout << "Enter temperature: ";
            cin >> temperature;
            cout << "Enter wind speed: ";
            cin >> windSpeed;
            Location* location = locationManager.findLocation(name);
            if (location) {
                vector<WeatherVariable> weatherVariables = {
                    WeatherVariable("Temperature", temperature),
                    WeatherVariable("Wind Speed", windSpeed)
                };
                exportToCSV(*location, weatherVariables, "data.csv");
                exportToJSON(*location, weatherVariables, "data.json");
                exportToTXT(*location, weatherVariables, "data.txt");
            }
            else {
                cout << "Location not found." << endl;
            }
            break;
        }
        case 8:
            return 0;
        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    }

    return 0;
}
