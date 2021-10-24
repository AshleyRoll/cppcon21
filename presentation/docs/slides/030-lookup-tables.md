## Lookup Tables

Lets make a lookup table that does linear interpolation.

- Warning: constexpr &lt;cmath&gt; is a gcc extension [P1383]
  <!-- .element: class="fragment" -->

Note:
- [P1383] More constexpr for &lt;cmath&gt; and &lt;complex&gt;
- Concerns over error handling, rounding and potential differences
  compile-time vs run-time values
- constexpr math on embedded would be very useful



<!-- down -->
- Table of sin(x) where x is in degrees, not radians
- Reusable type, just plug in a different function

```C++
constexpr float radians(float degrees) {
    return (std::numbers::pi/180.0)*degrees;
}
constexpr float sine_deg(float degrees ) {
    return std::sin(radians(degrees));
}

constexpr auto DegreeSineTable =
  LerpTable<float, 32>::make_table(0.0f, 90.0f, sine_deg);

static_assert(DegreeSineTable(30.0f) >= 0.499f
           && DegreeSineTable(30.0f) <= 0.501f);
```

Note:
- Control the number of entries with NTTP - affects precision




<!-- down -->
```C++
template<typename T, std::size_t NUM_ENTRIES>
struct LerpTable
{
    constexpr T operator()(T in) const
    { ... }

    constexpr static LerpTable make_table(
        T min, T max, T (&function)(T)
    )
    { ...  }

private:
    struct Entry
    {
        T input;
        T output;
    };

    std::array<Entry, NUM_ENTRIES> entries;
}
```
<!-- .element: class="r-stretch" -->



<!-- down -->
```C++
constexpr static LerpTable make_table(
    T min, T max, T (&function)(T) // or: auto function
)
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
```
<!-- .element: class="r-stretch" -->

Note:
- explicitly fill first and last entry avoid rounding on these values
- Function reference could be `auto function`, but doesn't document
  for the caller what is expected
- Use concepts



<!-- down -->
```C++
constexpr T operator()(T in) const
{
    auto const clamped_in = std::clamp(
        in,
        entries[0].input,
        entries[NUM_ENTRIES-1].input);

    auto entry_itr = std::lower_bound(
        entries.begin(), entries.end(), clamped_in,
        [](auto const &e, auto const &v) { return e.input < v;});

    auto const entry = *entry_itr;

    if(entry.input == clamped_in) {
        return entry.output; // handle exact match / first entry
    }

    auto const prev_entry = *std::prev(entry_itr);
    auto const t = (clamped_in - prev_entry.input)
                   / (entry.input - prev_entry.input);

    return std::lerp(prev_entry.output, entry.output, t);
}
```
<!-- .element: class="r-stretch" -->

Note:
- Clamp input to defined range, could error
- lower bound - first element that is __not less than__
  - will return first entry on 0.0f
  - special case exact match to avoid the LERP
    and handle first element where no previous

