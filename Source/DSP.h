#pragma once

#include <algorithm>
#include <cmath>
#include <complex>

namespace prisma
{
    constexpr int numBands = 6;

    enum BandType { Bell = 0, LowShelf = 1, HighShelf = 2, HighPass = 3, LowPass = 4 };

    struct Coeffs
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
    };

    struct BandState
    {
        double z1 = 0.0, z2 = 0.0;
    };

    // Filtros biquad (formulas RBJ Audio EQ Cookbook)
    inline Coeffs makeCoeffs (int type, double fs, double freq, double gainDb, double q)
    {
        constexpr double twoPi = 6.283185307179586;

        freq = std::clamp (freq, 10.0, fs * 0.49);
        q = std::max (q, 0.05);

        const double w0 = twoPi * freq / fs;
        const double cs = std::cos (w0);
        const double sn = std::sin (w0);
        const double alpha = sn / (2.0 * q);
        const double A = std::pow (10.0, gainDb / 40.0);
        const double beta = 2.0 * std::sqrt (A) * alpha;

        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a0 = 1.0, a1 = 0.0, a2 = 0.0;

        switch (type)
        {
            case LowShelf:
                b0 = A * ((A + 1.0) - (A - 1.0) * cs + beta);
                b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cs);
                b2 = A * ((A + 1.0) - (A - 1.0) * cs - beta);
                a0 = (A + 1.0) + (A - 1.0) * cs + beta;
                a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cs);
                a2 = (A + 1.0) + (A - 1.0) * cs - beta;
                break;

            case HighShelf:
                b0 = A * ((A + 1.0) + (A - 1.0) * cs + beta);
                b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cs);
                b2 = A * ((A + 1.0) + (A - 1.0) * cs - beta);
                a0 = (A + 1.0) - (A - 1.0) * cs + beta;
                a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cs);
                a2 = (A + 1.0) - (A - 1.0) * cs - beta;
                break;

            case HighPass:
                b0 = (1.0 + cs) * 0.5;
                b1 = -(1.0 + cs);
                b2 = (1.0 + cs) * 0.5;
                a0 = 1.0 + alpha;
                a1 = -2.0 * cs;
                a2 = 1.0 - alpha;
                break;

            case LowPass:
                b0 = (1.0 - cs) * 0.5;
                b1 = 1.0 - cs;
                b2 = (1.0 - cs) * 0.5;
                a0 = 1.0 + alpha;
                a1 = -2.0 * cs;
                a2 = 1.0 - alpha;
                break;

            case Bell:
            default:
                b0 = 1.0 + alpha * A;
                b1 = -2.0 * cs;
                b2 = 1.0 - alpha * A;
                a0 = 1.0 + alpha / A;
                a1 = -2.0 * cs;
                a2 = 1.0 - alpha / A;
                break;
        }

        Coeffs c;
        c.b0 = b0 / a0;
        c.b1 = b1 / a0;
        c.b2 = b2 / a0;
        c.a1 = a1 / a0;
        c.a2 = a2 / a0;
        return c;
    }

    // Forma transpuesta directa II
    inline double process (const Coeffs& c, BandState& s, double x) noexcept
    {
        const double y = c.b0 * x + s.z1;
        s.z1 = c.b1 * x - c.a1 * y + s.z2;
        s.z2 = c.b2 * x - c.a2 * y;
        return y;
    }

    // Respuesta en magnitud (dB) a una frecuencia, para dibujar la curva
    inline double magnitudeDb (const Coeffs& c, double freq, double fs)
    {
        constexpr double twoPi = 6.283185307179586;
        const double w = twoPi * freq / fs;
        const std::complex<double> z1 = std::polar (1.0, -w);
        const std::complex<double> z2 = z1 * z1;
        const std::complex<double> num = c.b0 + c.b1 * z1 + c.b2 * z2;
        const std::complex<double> den = 1.0 + c.a1 * z1 + c.a2 * z2;
        return 20.0 * std::log10 (std::abs (num) / std::abs (den) + 1.0e-12);
    }
}
