#pragma once

#include "DSP.h"

namespace prisma
{
    struct PresetBand
    {
        int type;
        float freq;
        float gain;
        float q;
        bool on;
    };

    struct Preset
    {
        const char* category;
        const char* name;
        PresetBand bands[numBands];
    };

    // 6 bandas por preset, en el mismo orden que las bandas del plugin (1 a 6).
    // Tipos: 0 Campana, 1 Shelf grave, 2 Shelf agudo, 3 Paso alto, 4 Paso bajo
    inline const Preset* getFactoryPresets (int& count)
    {
        static const Preset presets[] =
        {
            // ---- Guitarra electrica ----
            { "Guitarra electrica", "Rock rítmica",
              { { HighPass, 90.0f, 0.0f, 0.7f, true }, { Bell, 250.0f, -2.5f, 1.1f, true },
                { Bell, 550.0f, -3.0f, 1.4f, true }, { Bell, 1600.0f, 2.0f, 1.0f, true },
                { Bell, 3200.0f, 2.5f, 0.9f, true }, { HighShelf, 7000.0f, 1.5f, 0.7f, true } } },
            { "Guitarra electrica", "Lead solista",
              { { HighPass, 120.0f, 0.0f, 0.7f, true }, { Bell, 400.0f, -2.0f, 1.2f, true },
                { Bell, 900.0f, 1.5f, 1.0f, true }, { Bell, 2500.0f, 3.5f, 1.0f, true },
                { Bell, 4500.0f, 1.5f, 1.2f, true }, { HighShelf, 9000.0f, 2.0f, 0.7f, true } } },
            { "Guitarra electrica", "Limpia brillante",
              { { HighPass, 100.0f, 0.0f, 0.7f, true }, { Bell, 300.0f, -1.5f, 1.0f, true },
                { Bell, 1000.0f, 0.0f, 1.0f, false }, { Bell, 2800.0f, 2.5f, 1.1f, true },
                { HighShelf, 6000.0f, 3.0f, 0.7f, true }, { HighShelf, 12000.0f, 1.5f, 0.7f, true } } },

            // ---- Guitarra acustica ----
            { "Guitarra acustica", "Rasgueo cálido",
              { { HighPass, 80.0f, 0.0f, 0.7f, true }, { LowShelf, 150.0f, 1.5f, 0.7f, true },
                { Bell, 350.0f, -2.5f, 1.2f, true }, { Bell, 2000.0f, 1.5f, 1.0f, true },
                { Bell, 5000.0f, 2.0f, 1.0f, true }, { HighShelf, 10000.0f, 2.5f, 0.7f, true } } },
            { "Guitarra acustica", "Fingerstyle detallado",
              { { HighPass, 90.0f, 0.0f, 0.7f, true }, { Bell, 220.0f, -1.5f, 1.0f, true },
                { Bell, 500.0f, -2.0f, 1.3f, true }, { Bell, 3000.0f, 2.0f, 1.1f, true },
                { Bell, 6500.0f, 2.5f, 1.0f, true }, { HighShelf, 11000.0f, 2.0f, 0.7f, true } } },

            // ---- Voz femenina ----
            { "Voz femenina", "Pop moderno",
              { { HighPass, 110.0f, 0.0f, 0.7f, true }, { Bell, 300.0f, -2.0f, 1.2f, true },
                { Bell, 800.0f, -1.5f, 1.3f, true }, { Bell, 3000.0f, 3.0f, 1.1f, true },
                { Bell, 6000.0f, 2.0f, 1.0f, true }, { HighShelf, 11000.0f, 2.5f, 0.7f, true } } },
            { "Voz femenina", "Balada íntima",
              { { HighPass, 100.0f, 0.0f, 0.7f, true }, { LowShelf, 200.0f, 1.0f, 0.7f, true },
                { Bell, 500.0f, -1.5f, 1.2f, true }, { Bell, 2500.0f, 2.0f, 1.0f, true },
                { Bell, 5500.0f, 1.5f, 1.1f, true }, { HighShelf, 10000.0f, 1.5f, 0.7f, true } } },
            { "Voz femenina", "Presencia radio",
              { { HighPass, 130.0f, 0.0f, 0.7f, true }, { Bell, 350.0f, -3.0f, 1.1f, true },
                { Bell, 1000.0f, -1.0f, 1.3f, true }, { Bell, 3500.0f, 3.5f, 1.0f, true },
                { Bell, 7000.0f, 2.5f, 0.9f, true }, { HighShelf, 12000.0f, 3.0f, 0.7f, true } } },

            // ---- Voz masculina ----
            { "Voz masculina", "Grave y cálida",
              { { HighPass, 85.0f, 0.0f, 0.7f, true }, { LowShelf, 180.0f, 2.0f, 0.7f, true },
                { Bell, 450.0f, -2.5f, 1.2f, true }, { Bell, 2200.0f, 2.0f, 1.0f, true },
                { Bell, 5000.0f, 1.5f, 1.1f, true }, { HighShelf, 9500.0f, 1.5f, 0.7f, true } } },
            { "Voz masculina", "Podcast claro",
              { { HighPass, 100.0f, 0.0f, 0.7f, true }, { Bell, 300.0f, -2.0f, 1.1f, true },
                { Bell, 900.0f, -1.5f, 1.3f, true }, { Bell, 3000.0f, 2.5f, 1.0f, true },
                { Bell, 6000.0f, 1.5f, 1.0f, true }, { HighShelf, 10000.0f, 1.0f, 0.7f, true } } },
            { "Voz masculina", "Rock agresiva",
              { { HighPass, 110.0f, 0.0f, 0.7f, true }, { Bell, 400.0f, -3.0f, 1.2f, true },
                { Bell, 1200.0f, 1.5f, 1.2f, true }, { Bell, 3200.0f, 3.0f, 1.0f, true },
                { Bell, 6500.0f, 2.5f, 0.9f, true }, { HighShelf, 11000.0f, 2.0f, 0.7f, true } } },

            // ---- Bateria / percusion ----
            { "Bateria", "Bombo punch",
              { { HighPass, 30.0f, 0.0f, 0.7f, true }, { Bell, 60.0f, 3.0f, 1.0f, true },
                { Bell, 400.0f, -4.0f, 1.3f, true }, { Bell, 2500.0f, 2.5f, 1.0f, true },
                { HighShelf, 8000.0f, 1.5f, 0.7f, true }, { Bell, 100.0f, 0.0f, 1.0f, false } } },
            { "Bateria", "Caja con cuerpo",
              { { HighPass, 60.0f, 0.0f, 0.7f, true }, { Bell, 200.0f, 2.0f, 1.1f, true },
                { Bell, 500.0f, -2.5f, 1.3f, true }, { Bell, 3000.0f, 3.0f, 1.0f, true },
                { HighShelf, 7000.0f, 2.5f, 0.7f, true }, { Bell, 1000.0f, 0.0f, 1.0f, false } } },
            { "Bateria", "Overheads brillantes",
              { { HighPass, 150.0f, 0.0f, 0.7f, true }, { Bell, 400.0f, -2.0f, 1.2f, true },
                { Bell, 3000.0f, 1.5f, 1.0f, true }, { HighShelf, 8000.0f, 3.5f, 0.7f, true },
                { HighShelf, 14000.0f, 2.0f, 0.7f, true }, { Bell, 1000.0f, 0.0f, 1.0f, false } } },

            // ---- Bajo ----
            { "Bajo", "Eléctrico moderno",
              { { HighPass, 35.0f, 0.0f, 0.7f, true }, { LowShelf, 80.0f, 2.5f, 0.7f, true },
                { Bell, 300.0f, -2.5f, 1.3f, true }, { Bell, 800.0f, -1.5f, 1.2f, true },
                { Bell, 1500.0f, 2.0f, 1.0f, true }, { HighShelf, 4000.0f, 1.5f, 0.7f, true } } },
            { "Bajo", "Vintage cálido",
              { { HighPass, 30.0f, 0.0f, 0.7f, true }, { LowShelf, 100.0f, 3.0f, 0.7f, true },
                { Bell, 350.0f, -1.5f, 1.1f, true }, { Bell, 900.0f, -2.0f, 1.2f, true },
                { Bell, 2000.0f, 1.0f, 1.0f, true }, { HighShelf, 5000.0f, 0.5f, 0.7f, true } } },
            { "Bajo", "Slap definido",
              { { HighPass, 45.0f, 0.0f, 0.7f, true }, { Bell, 120.0f, 1.5f, 1.0f, true },
                { Bell, 400.0f, -3.0f, 1.3f, true }, { Bell, 1200.0f, 2.5f, 1.1f, true },
                { Bell, 3000.0f, 2.0f, 1.0f, true }, { HighShelf, 6000.0f, 1.5f, 0.7f, true } } },

            // ---- Piano ----
            { "Piano", "Acústico natural",
              { { HighPass, 40.0f, 0.0f, 0.7f, true }, { LowShelf, 150.0f, 1.0f, 0.7f, true },
                { Bell, 400.0f, -1.5f, 1.1f, true }, { Bell, 2000.0f, 1.0f, 1.0f, true },
                { Bell, 5000.0f, 1.5f, 1.0f, true }, { HighShelf, 9000.0f, 1.5f, 0.7f, true } } },
            { "Piano", "Balada brillante",
              { { HighPass, 50.0f, 0.0f, 0.7f, true }, { Bell, 300.0f, -2.0f, 1.1f, true },
                { Bell, 1000.0f, 0.5f, 1.0f, true }, { Bell, 3000.0f, 2.0f, 1.0f, true },
                { HighShelf, 7000.0f, 2.5f, 0.7f, true }, { HighShelf, 12000.0f, 1.5f, 0.7f, true } } },

            // ---- Metales ----
            { "Metales", "Trompeta y saxo",
              { { HighPass, 100.0f, 0.0f, 0.7f, true }, { Bell, 350.0f, -2.0f, 1.2f, true },
                { Bell, 1200.0f, 2.0f, 1.0f, true }, { Bell, 3000.0f, 2.5f, 1.0f, true },
                { Bell, 6000.0f, 1.5f, 1.1f, true }, { HighShelf, 10000.0f, 1.5f, 0.7f, true } } },
            { "Metales", "Sección completa",
              { { HighPass, 80.0f, 0.0f, 0.7f, true }, { Bell, 300.0f, -1.5f, 1.1f, true },
                { Bell, 1000.0f, 1.5f, 1.0f, true }, { Bell, 2500.0f, 2.0f, 1.0f, true },
                { Bell, 5500.0f, 2.0f, 1.0f, true }, { HighShelf, 9000.0f, 1.0f, 0.7f, true } } },
        };

        count = (int) (sizeof (presets) / sizeof (Preset));
        return presets;
    }
}
