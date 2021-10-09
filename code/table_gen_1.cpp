#include <array>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

#include <iostream>
#include <iomanip>


template<std::size_t NUM_ENTRIES>
struct LerpTable
{
    constexpr static auto NumEntries = NUM_ENTRIES;

    constexpr float operator()(float in) const
    {
        // force bounds
        auto const clamped_in = std::clamp(in, entries[0].input, entries[NumEntries-1].input);

        auto entry = std::lower_bound(entries.begin(), entries.end(), clamped_in,
            [](auto const &e, auto const &v) { return e.input < v;});

        if((*entry).input == clamped_in) {
            return (*entry).output;
        } else {
            auto prev_entry = std::prev(entry);
            auto const t = (clamped_in - (*prev_entry).input) / ((*entry).input - (*prev_entry).input);
            return std::lerp((*prev_entry).output, (*entry).output, t);
        }
    }

    struct Entry
    {
        float input;
        float output;
    };

    // assume linearly spaced between min/max
    std::array<Entry, NUM_ENTRIES> entries;
};

constexpr float radians(float degrees)
{
    return (std::numbers::pi/180.0)*degrees;
}


// build a 0-90 degree sine table with the provided number of entries
template<std::size_t NUM_ENTRIES>
constexpr auto sine_degrees()
{
    LerpTable<NUM_ENTRIES> table;

    table.entries[0] = {0.0f, std::sin(radians(0.0f))};
    table.entries[table.NumEntries-1] = {90.0f, std::sin(radians(90.0f))};

    float const step = 90.0f / table.NumEntries;
    for(std::size_t i = 1; i < table.NumEntries - 1; ++i) {
        float a = step * i;
        table.entries[i] = {a, std::sin(radians(a))};
    }

    return table;
}

constinit auto SineTable = sine_degrees<20>();

int main()
{
    std::vector<float> angles{0.0f, 20.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f, 90.0f};

    for(auto const a : angles ) {
        auto const sin = std::sin(radians(a));
        auto const tab = SineTable(a);
        auto const diff = sin - tab;
        auto const percent = (sin != 0.0f) ? (diff / sin * 100.0f) : 0.0f;

        std::cout << std::setprecision(std::numeric_limits<float>::digits10 + 1)
            << a << ": " << sin << " " << tab << " (" << diff << ", " << percent << "%)\n";
    }

    return 0;
}
