// https://github.com/jimmyberg/LowPassFilter

#pragma once

#include <cmath>

class LowPassFilter {
  public:
    // constructors
    LowPassFilter();
    LowPassFilter(float iCutOffFrequency, float iDeltaTime);
    // functions
    inline float update(float input) { return output += (input - output) * ePow; }

    float update(float input, float deltaTime, float cutoffFrequency);

    // get and configure funtions
    float getOutput() const { return output; }
    void reconfigureFilter(float deltaTime, float cutoffFrequency);

  private:
    float output;
    float ePow;
};
