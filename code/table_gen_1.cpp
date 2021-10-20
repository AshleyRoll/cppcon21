#include <array>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

#include <iostream>
#include <iomanip>


template<typename T, std::size_t NUM_ENTRIES>
struct LerpTable
{
    constexpr T operator()(T in) const
    {
        auto const clamped_in = std::clamp(in, entries[0].input, entries[NUM_ENTRIES-1].input);

        auto entry_itr = std::lower_bound(entries.begin(), entries.end(), clamped_in,
            [](auto const &e, auto const &v) { return e.input < v;});

        auto const entry = *entry_itr;

        if(entry.input == clamped_in) {
            return entry.output; // handle exact match / first entry
        }

        auto const prev_entry = *std::prev(entry_itr);
        auto const t = (clamped_in - prev_entry.input) / (entry.input - prev_entry.input);
        return std::lerp(prev_entry.output, entry.output, t);
    }

    constexpr static LerpTable make_table(T min, T max, T (&function)(T))
    {
        LerpTable table;

        // fill min/max value explicitly
        table.entries[0] = {min, function(min)};
        table.entries[NUM_ENTRIES-1] = {max, function(max)};

        // fill the other entries
        T const step = (max - min) / NUM_ENTRIES;
        for(std::size_t i = 1; i < NUM_ENTRIES-1; ++i) {
            T const a = step * i;
            table.entries[i] = {a, function(a)};
        }

        return table;
    }

private:
    struct Entry
    {
        T input;
        T output;
    };

    std::array<Entry, NUM_ENTRIES> entries;
};

constexpr float radians(float degrees) { return (std::numbers::pi/180.0)*degrees; }
constexpr float sine_deg(float degrees ) { return std::sin(radians(degrees)); }

// sine table, using degrees as input from 0-90 degrees
constinit const auto DegreeSineTable = LerpTable<float, 32>::make_table(0.0f, 90.0f, sine_deg);

int main()
{
    std::vector<float> angles{0.0f, 20.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f, 90.0f};

    for(auto const a : angles ) {
        auto const sin = std::sin(radians(a));
        auto const tab = DegreeSineTable(a);
        auto const diff = sin - tab;

        std::cout << std::setprecision(std::numeric_limits<float>::digits10 + 1)
            << a << ": " << sin << " " << tab << " (" << diff << ")\n";
    }

    return 0;
}
