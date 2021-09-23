#include <cstdio>
#include <cstdint>
#include <array>

#include "usbdescriptor.h"

constinit auto const usbConfiguration1Descriptor = usb::descriptor::MakeConfigurationDescriptor([]() {
    using namespace usb::descriptor;

    return Configuration{
            1,
            3,
            false,
            false,
            100,
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


void DumpHexBlock(std::span<std::uint8_t const> buffer)
{
    constexpr static auto hex = "0123456789ABCDEF";

    for(auto i = 0; i < buffer.size(); ++i)
    {
        if(i % 8 == 0)
            putchar('\n');
        else
            putchar(' ');

        const auto b = buffer[i];

        putchar(hex[(b >> 4)]);
        putchar(hex[(b & 0x0F)]);
    }

    putchar('\n');
}



int main()
{
    DumpHexBlock(usbConfiguration1Descriptor);
}
