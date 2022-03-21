#include "sinusoid.h"

#include <math.h>

static double
get_sinusoid_phase_increment(const sinusoid sinusoid, const int frame_size, const double sample_rate)
{
    return 2 * M_PI * sinusoid.frequency * frame_size / sample_rate;
}

sinusoid create_sinusoid(const double amplitude, const double frequency)
{
    sinusoid sinusoid;
    sinusoid.amplitude = amplitude;
    sinusoid.frequency = frequency;
    return sinusoid;
}

double get_sinusoid_value(const sinusoid sinusoid, const int frame, const int frame_size, const int sample, const double sample_rate)
{
    return sinusoid.amplitude * sin(2 * M_PI * sinusoid.frequency * (sample / sample_rate) + frame * get_sinusoid_phase_increment(sinusoid, frame_size, sample_rate));
}
