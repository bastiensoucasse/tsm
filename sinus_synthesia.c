#include <math.h>

/**
 * @brief Samples a sinusoid into an array.
 * 
 * @param sinus The array of samples to fill.
 * @param num_samples The number of samples (size).
 * @param sampling_rate The sample rate (number of samples each second).
 * @param amplitude The sinusoid amplitude (peak deviation from zero).
 * @param frequency The sinusoid frequency (number of oscillations each second of time, reciprocal of the period).
 * @param phase The sinusoid oscillation position at t = 0.
 */
void sinus_synthesia(float *sinus, int num_samples, int sampling_rate, float amplitude, float frequency, float phase)
{
    for (int sample = 0; sample < num_samples; sample++)
        sinus[sample] = amplitude * sin(2 * M_PI * frequency * (sample / sampling_rate) + phase);
}
