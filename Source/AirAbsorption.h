/*
  ==============================================================================

    AirAbsorption.h
    Created: 3 Apr 2024 9:50:47am
    @Author:  regnier
    @Brief: Approximative Air Absorption method. This simply gives the cutoff frequency for a 1-pole LP, depending on distance and atmospheric conditions
    from : https://computingandrecording.wordpress.com/2017/07/05/approximating-atmospheric-absorption-with-a-simple-filter/

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class AirAbsorption
{
    public:

        void FilterCutoffSolver(
            const double humidity_percent,
            const double temperature_farenheit,
            const double pressure_pascals);

        double cutoffSolve(const double distance, const double cutoff_gain);

        const double kPressureSeaLevelPascals = 101325.0;
        const double kReferenceAirTemperature = 293.15;

    private:
       
        double CelsiusToKelvin(const double celsius);

        double HumidityConcentration(
            const double humidity_percent, // 0 to 100.0
            const double temperature_kelvin,
            const double pressure_normalized);

        double NitrogenRelaxationFrequency(
            const double humidity_concentration,
            const double temp_normalized,
            double pressure_normalized);

        double OxygenRelaxationFrequency(
            double humidity_concentration,
            double pressure_normalized);

        double AbsorptionCoefficient(
            const double frequency_hz,
            const double humidity_percent,
            const double temperature_celsius,
            const double pressure_pascals);

        double FindFirstRoot(double a, double b, double c, double d);

        double nitrogen_relax_freq;
        double oxygen_relax_freq;
        double a1, a2, a3;
};