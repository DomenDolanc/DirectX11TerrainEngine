#include "pch.h"
#include "PerlinNoise.h"

using namespace Terrain_engine;

double PerlinNoise::Noise(int i, int x, int y) {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    int a = m_Primes[i][0], b = m_Primes[i][1], c = m_Primes[i][2];
    int t = (n * (n * n * a + b) + c) & 0x7fffffff;
    return 1.0 - (double)(t) / 1073741824.0;
}

double PerlinNoise::SmoothedNoise(int i, int x, int y) {
    double corners = (Noise(i, x - 1, y - 1) + Noise(i, x + 1, y - 1) +
        Noise(i, x - 1, y + 1) + Noise(i, x + 1, y + 1)) / 16,
        sides = (Noise(i, x - 1, y) + Noise(i, x + 1, y) + Noise(i, x, y - 1) +
            Noise(i, x, y + 1)) / 8,
        center = Noise(i, x, y) / 4;
    return corners + sides + center;
}

double PerlinNoise::Interpolate(double a, double b, double x) {
    double ft = x * 3.1415927,
        f = (1 - cos(ft)) * 0.5;
    return  a * (1 - f) + b * f;
}

double PerlinNoise::InterpolatedNoise(int i, double x, double y) {
    int integer_X = x;
    double fractional_X = x - integer_X;
    int integer_Y = y;
    double fractional_Y = y - integer_Y;

    double v1 = SmoothedNoise(i, integer_X, integer_Y),
        v2 = SmoothedNoise(i, integer_X + 1, integer_Y),
        v3 = SmoothedNoise(i, integer_X, integer_Y + 1),
        v4 = SmoothedNoise(i, integer_X + 1, integer_Y + 1),
        i1 = Interpolate(v1, v2, fractional_X),
        i2 = Interpolate(v3, v4, fractional_X);
    return Interpolate(i1, i2, fractional_Y);
}

double PerlinNoise::GenerateValue(double x, double y) {
    double total = 0;
    double frequency = pow(2, m_NumOfOctaves);
    double amplitude = m_Amplitude;

    for (int i = 0; i < m_NumOfOctaves; ++i) {
        frequency /= 2;
        amplitude *= m_Persistence;
        total += InterpolatedNoise((m_PrimeIndex + i) % maxPrimeIndex,
            x / frequency, y / frequency) * amplitude;
    }
    return total / frequency;
}

int PerlinNoise::getNumOfOctaves()
{
    return m_NumOfOctaves;
}

void PerlinNoise::setNumOfOctaves(int value)
{
    m_NumOfOctaves = value;
}

double PerlinNoise::getPersistance()
{
    return m_Persistence;
}

void PerlinNoise::setPersistance(double value)
{
    m_Persistence = value;
}

double PerlinNoise::getAmplitude()
{
    return m_Amplitude;
}

void PerlinNoise::setAmplitude(double value)
{
    m_Amplitude = value;
}

int Terrain_engine::PerlinNoise::getPrimeIndex()
{
    return m_PrimeIndex;
}

void Terrain_engine::PerlinNoise::setPrimeIndex(int value)
{
    m_PrimeIndex = value;
}

