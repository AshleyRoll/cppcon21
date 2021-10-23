#include <cstdint>
#include <array>
#include <algorithm>

enum class WatchDogMode
{
    Disabled = 0,
    Enabled_1ms = 1,
    Enabled_10ms = 2,
    Enabled_100ms = 3,
};

enum class OscillatorMode
{
    InternalRC = 1,
    ExternalRC = 2,
    Crystal = 3,
    ExternalClock = 4
};

// Simple Config builder that sets watchdog and oscillator
class ConfigBuilder
{
  public:
    constexpr void set_watchdog(WatchDogMode wtd)
    {
        m_Wdt = wtd;
    }

    constexpr void set_oscillator(OscillatorMode osc)
    {
        m_Osc = osc;
    }

    constexpr auto build()
    {
        // Serialise all the registers in correct order and in to the correct bit locations
        // without relying on mapping structs and packing correctly
        // Lets pretend the registers are 32 bits, and there 2 of them
        std::array<std::uint32_t, 2> registers;

        auto wdt = static_cast<std::uint32_t>(m_Wdt);
        auto osc = static_cast<std::uint32_t>(m_Osc);

        // lets pretend we need values and their complement
        registers[0] = (wdt << 24u) | (~wdt & 0x0000'00FF);
        registers[1] = (osc << 24u) | (~osc & 0x0000'00FF);

        return registers;
    }

private:
    WatchDogMode m_Wdt {WatchDogMode::Disabled};            // configure safe defaults
    OscillatorMode m_Osc {OscillatorMode::InternalRC};
};

// Now lets use the configuration builder to configure the processor
//
// We simply constinit the value and place it into a named section.
// Then use the linker script to map that into the correct Flash Memory
// address.
//
// NO compiler extensions or paragmas needed to set up the processor hardware now.
[[gnu::section(".config_registers"), gnu::used]]
constinit const auto CONFIG_REGISTERS = []{
    ConfigBuilder cfg;

    cfg.set_watchdog(WatchDogMode::Enabled_10ms);
    cfg.set_oscillator(OscillatorMode::Crystal);

    return cfg.build();
}();

int main()
{
    return 0;
}
