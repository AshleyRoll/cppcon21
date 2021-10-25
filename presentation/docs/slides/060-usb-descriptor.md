
## USB descriptors

Lets make a USB Configuration descriptor

- https://github.com/AshleyRoll/cpp_usbdescriptor
- Partial implementation so far
- Simple Reference for USB protocol
  https://www.beyondlogic.org/usbnutshell/usb1.shtml
- This is a brief overview, check out the code for implementation details
  <!-- .element: class="fragment" -->
- Most complex example - uses variadic templates, std::tuple and
  constexpr functions to deal with different sized interfaces
  <!-- .element: class="fragment" -->



<!-- down -->
### Configuration Descriptor

- Tells the host about interfaces and endpoints
  <!-- .element: class="fragment" -->
- Interfaces make "functionality"
  <!-- .element: class="fragment" -->
- Endpoints are data transfers addresses
  <!-- .element: class="fragment" -->
- Variable number of interfaces
  <!-- .element: class="fragment" -->
- Each interface has variable number of endpoints
  <!-- .element: class="fragment" -->



<!-- down -->
```C++
constinit auto const Descriptor1
 = usb::descriptor::MakeConfigurationDescriptor([]() {
    using namespace usb::descriptor;

    return Configuration{
            1,      // config number
            3,      // string identifier
            false,  // selfPowered
            false,  // remoteWakeup
            100,    // 200mA (units of 2mA)
            define_vendor_specific_interface(
                1,
                BulkEndpoint{EndpointDirection::Out, 1, 512},
                InterruptEndpoint{EndpointDirection::In, 1, 512, 1}
            ),
            define_vendor_specific_interface(
                2,
                BulkEndpoint{EndpointDirection::Out, 1, 512},
                BulkEndpoint{EndpointDirection::In, 1, 512},
                BulkEndpoint{EndpointDirection::In, 2, 512},
                BulkEndpoint{EndpointDirection::In, 3, 512}
            )
    };
});
```
<!-- .element: class="r-stretch" -->



<!-- down -->

```text
$ objdump -s --section=.descriptor usbdescriptors

usbdescriptors:     file format elf64-x86-64

Contents of section .descriptor:
 2020 09024500 02010380 64090400 0002ffff  ..E.....d.......
 2030 ff010705 01020002 00070581 03000201  ................
 2040 09040000 04ffffff 02070501 02000200  ................
 2050 07058102 00020007 05820200 02000705  ................
 2060 83020002 00
 ```
 - Placed `Descriptor1` into `.descritor` section to help dump it



<!-- down -->
```C++
template<std::size_t ... Sizes>
class Configuration
{
    constexpr Configuration(
        std::uint8_t  configurationNumber,
        std::uint8_t stringIdentifier,
        bool selfPowered,
        bool remoteWakeup,
        std::uint8_t maxPower_2mAUnits,
        Interface<Sizes>... interfaces
    )
    { /* ... */ }
};
```



<!-- down -->
```C++
template<std::size_t NumEndpoints>
class Interface
{
    constexpr Interface(
        std::uint8_t interfaceClass, std::uint8_t interfaceSubClass,
        std::uint8_t interfaceProtocol, std::uint8_t stringIdentifier,
        std::array<Endpoint, NumEndpoints> endpoints
    )
    { /* ... */ }
};

class Endpoint
{
    // Endpoint is a literal type, no need to have a builder
    constexpr Endpoint(
        EndpointDirection direction, std::uint8_t address,
        EndpointTransfer transfer,
        EndpointSynchronisation synchronisation,
        EndpointUsage usage, std::uint16_t maxPacketSize,
        std::uint8_t interval
    ) { /* ... */ }
};

// derived class for BulkEndpoint, InterruptEndpoint etc...
```
<!-- .element: class="r-stretch" -->




<!-- down -->
```C++
// Specific interface type helper function
template<typename ... TEPs>
constexpr auto define_vendor_specific_interface(
    std::uint8_t stringIdentifier,
    TEPs ... endpoints)
{
    return define_interface(
        0xFF, 0xFF, 0xFF, stringIdentifier,
        endpoints...);
}

// General Helper functions to create interface
template<typename ... TEPs>
constexpr auto define_interface(
    std::uint8_t interfaceClass, std::uint8_t interfaceSubClass,
    std::uint8_t interfaceProtocol, std::uint8_t stringIdentifier,
    TEPs ... endpoints)
{
   return Interface<sizeof...(TEPs)>{
       interfaceClass, interfaceSubClass,
       interfaceProtocol, stringIdentifier,
       std::array<Endpoint, sizeof...(TEPs)>{ endpoints... }
   };
}
```
<!-- .element: class="r-stretch" -->



<!-- down -->
### Rendering the descriptor

- Each class has a length() method
  <!-- .element: class="fragment" -->
- Each class has a render() method taking std::span
  <!-- .element: class="fragment" -->
- the MakeConfigurationDescriptor() function then:
  <!-- .element: class="fragment" -->
  - Calculates the required buffer size
  <!-- .element: class="fragment" -->
  - calls the render method to fill an array
  <!-- .element: class="fragment" -->
- Use std::tuple of interfaces of varing number of endpoints
  <!-- .element: class="fragment" -->
- Use std::apply to fold over tuple for length/render
  <!-- .element: class="fragment" -->



<!-- down -->
```C++
// Concept to enforce a lambda
template<typename T, std::size_t ... Sizes>
concept CallableGivesConfiguration = requires(T t)
{
    t();            // is callable
    // result is a Configuration<...>
    std::is_same_v<decltype(t()), Configuration<Sizes...>>;
};

// Rendering helper, pass in the lambda to generate the configuration
template<std::size_t ... Sizes>
consteval static auto MakeConfigurationDescriptor(
    CallableGivesConfiguration<Sizes...> auto makeConfigLambda)
{
    // build the configuration using the supplied lambda
    constexpr auto cfg = makeConfigLambda();
    constexpr auto len = cfg.length();

    std::array<std::uint8_t, len> data;

    cfg.Render(data);

    return data;
}
```
<!-- .element: class="r-stretch" -->



<!-- down -->
```C++
template<std::size_t ... Sizes>
class Configuration
{
    constexpr std::size_t length() const
    {
        return ConfigurationDescriptorSize + std::apply(
            [](auto && ... interfaces)
            {
                return (0 + ... + interfaces.length());
            },
            m_Interfaces
        );
    }

    std::tuple<Interface<Sizes>...> m_Interfaces;
};
```
<!-- .element: class="r-stretch" -->



<!-- down -->
```C++
template<std::size_t ... Sizes>
class Configuration
{
    constexpr void Render(std::span<std::uint8_t> buffer) const
    {
        // ... Render fixed Configuration data ...
        std::size_t location{ConfigurationDescriptorSize};
        std::apply(
            [&](auto && ... interfaces) {
                std::size_t index{0};
                auto render = [&](auto i) {
                    auto len = i.length();
                    i.Render(buffer.subspan(location, len), index);
                    location += len;
                };

                ((render(interfaces)), ...);
            }, m_Interfaces);
        // store lenght data into fixed Configuration section
        impl::write_le(buffer.subspan<2, 2>(), location);
    }
};
```
<!-- .element: class="r-stretch" -->

