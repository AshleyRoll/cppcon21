#include <iostream>
#include <iomanip>
#include <array>

#include "usbdescriptor.h"

static auto buildUsbDescriptor = []() {
    using namespace usb::descriptor;

    return Configuration{
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
};

constexpr auto usbDescriptor = []() {
    constexpr auto i = buildUsbDescriptor();
    constexpr auto l = i.length();

    std::array<uint8_t, l> data;

    i.Render(data, 1);
    return data;
}();


int main() {
    for(auto b : usbDescriptor)
    {
        std::cout << "0x" << std::hex << static_cast<uint32_t>(b) << std::dec << "\n";
    }
}
