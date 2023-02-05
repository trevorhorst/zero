#ifndef TOOLS_WEATHER_H
#define TOOLS_WEATHER_H

#include <stdint.h>
#include <math.h>

/**
 * @brief Calculates the dew point in Celsius
 * 
 * @param temp Ambient temperature in Celsius
 * @param rh Relative humidity
 * @return float Dew point in Celsius
 */
float calculate_dew_point(float temp, float rh);

/**
 * @brief Calculates the frost point in Celsius
 * 
 * @param temp Ambient temperature in Celsius
 * @param dew_point Dew point in Celsius
 * @return float Frost point in Celsius
 */
float calculate_frost_point(float temp, float dew_point);

/**
 * @brief Converts a given temperature from degrees Celsius to degrees Farenheit
 * 
 * @param temp Ambient temperature in Celsius
 * @return float Temperature in degrees Farenheit
 */
float convert_celsius_to_farenheit(float temp);

/**
 * @brief Converts a given temeprature from degress Farenheit to degress Celsius
 * 
 * @param temp Ambient temeprature in Farenheit
 * @return float Temperature in degrees Celsius
 */
float convert_farenheit_to_celsius(float temp);

#endif // TOOLS_WEATHER_H
