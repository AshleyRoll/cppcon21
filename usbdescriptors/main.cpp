#include <iostream>
#include <iomanip>

#include "usbdescriptor.h"

constexpr auto ep1 = []() {
    const utils::usb::BulkEndpoint e { utils::usb::EndpointDirection::Out, 1, 512 };
    std::array<std::uint8_t, e.DescriptorLength> descriptor;
    e.Render(descriptor);
    return descriptor;
}();


int main() {
    for(auto b : ep1)
    {
        std::cout << "0x" << std::hex << static_cast<uint32_t>(b) << std::dec << "\n";
    }
}
