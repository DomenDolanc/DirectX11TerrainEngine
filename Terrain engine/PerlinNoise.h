#pragma once
#define maxPrimeIndex 10

namespace Terrain_engine
{
    class PerlinNoise
    {
    public:

        double GenerateValue(double x, double y);

        int getNumOfOctaves();
        void setNumOfOctaves(int value);

        double getPersistance();
        void setPersistance(double value);

        double getAmplitude();
        void setAmplitude(double value);

    private:
        double InterpolatedNoise(int i, double x, double y);
        double Noise(int i, int x, int y);
        double SmoothedNoise(int i, int x, int y);
        double Interpolate(double a, double b, double x);

        int m_NumOfOctaves = 5;
        double m_Persistence = 0.5;
        double m_Amplitude = 2;

        int m_PrimeIndex = 0;

        int m_Primes[maxPrimeIndex][3] = {
          { 995615039, 600173719, 701464987 },
          { 831731269, 162318869, 136250887 },
          { 174329291, 946737083, 245679977 },
          { 362489573, 795918041, 350777237 },
          { 457025711, 880830799, 909678923 },
          { 787070341, 177340217, 593320781 },
          { 405493717, 291031019, 391950901 },
          { 458904767, 676625681, 424452397 },
          { 531736441, 939683957, 810651871 },
          { 997169939, 842027887, 423882827 }
        };
    };
}

