#include "core/tools/weather.h"

float calculate_dew_point(float temp, float rh)
{
    float a = 17.27;
    float b = 237.7;

    float alpha = ((a * temp) / (b + temp)) + log(rh / 100.0);
    return (b * alpha) / (a - alpha);
}

float calculate_frost_point(float temp, float dew_point)
{
    float dewpoint_k = 273.15 + dew_point;
    float temp_k = 273.15 + temp;
    float frostpoint_k = dewpoint_k - temp_k + 2671.02 / ((2954.61 / temp_k) + 2.193665 * log(temp_k) - 13.3448);
    return frostpoint_k - 273.15;
}

float convert_celsius_to_farenheit(float temp)
{
    return ((9 * temp) / 5) + 32;
}

float convert_farenheit_to_celsius(float temp)
{
    return (5 * (temp - 32)) / 9;

}