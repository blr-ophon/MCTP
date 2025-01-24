#include "fgen.h"

// Y(t) = Asin(wt)
static double FGen_SINE(Wave wave, double time) {
    return wave.amp * sin(2*M_PI * wave.freq * time);
}

static double FGen_SQUARE(Wave wave, double time) {
    return (FGen_SINE(wave, time) >= 0) ? wave.amp : -wave.amp;
}

static double FGen_TRIG(Wave wave, double time) {
    double period = 1.0 / wave.freq;

    // Find the current position within the period
    double t_mod = fmod(time, period);
    // Normalize t_mod to the range [0, 1]
    double normalized_time = t_mod / period;

    double triangle_wave;
    if (normalized_time < 0.5) {
        triangle_wave = 4.0 * normalized_time - 1.0;  // Rising slope
    } else {
        triangle_wave = 3.0 - 4.0 * normalized_time;  // Falling slope
    }

    return wave.amp * triangle_wave;
}

double (*FGen_WAVEFORM[])(Wave, double) = {
    FGen_SINE,
    FGen_SQUARE,
    FGen_TRIG
};


void FGen_out(float *sampleBuf, int bufSize, Wave wave, double sample_rate, int duration) {
    double period = 1/sample_rate;
    int samples_total = duration/period;

    for(int i = 0; i < samples_total; i++) {
        double time = i*period;
        //fprintf(f, "%f %f\n", time, FGen_WAVEFORM[wave.form](wave, time));
    }
}

void FGen_simple(float *sampleBuf, int bufSize, Wave wave, double sample_rate) {
    double period = 1/sample_rate;

    for(int i = 0; i < bufSize; i++) {
        double time = i*period;
        sampleBuf[i] = FGen_WAVEFORM[wave.form](wave, time);
    }
}
