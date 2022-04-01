#ifndef SINUSOID_H
#define SINUSOID_H

typedef struct sinusoid {
    double amplitude;
    double frequency;
} sinusoid;

/**
 * @brief Creates a sinusoid object.
 *
 * @param amplitude The amplitude of the sinusoid to create.
 * @param frequency The frequency of the sinusoid to create.
 * @return The sinusoid object.
 */
sinusoid create_sinusoid(const double amplitude, const double frequency);

/**
 * @brief Gets the sinusoid value.
 *
 * @param sinusoid The sinusoid to evaluate.
 * @param frame The frame number in which to find the sample.
 * @param frame_size The frame size.
 * @param sample The sample number.
 * @param sample_rate The sample rate.
 * @return The sinusoid value.
 */
double get_sinusoid_value(const sinusoid sinusoid, const int frame, const int frame_size, const int sample, const double sample_rate);

#endif // SINUSOID_H
