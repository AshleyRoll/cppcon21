#ifndef USB_USBDESCRIPTOR_H
#define USB_USBDESCRIPTOR_H


//
// This is a partial implementation, no support for:
//
//  - ideally, the USB standard Interface classes, subclasses and protocols would be
//    supported using strong types built on the underlying types here
//  - building string descriptors, but you can specify the index where used.
//  - no support to alternative interfaces
//  - Only have Bulk and Interrupt Endpoint Specialisation, others should be made,
//    but can use the Endpoint class to create them
//  - Interfaces are implicitly numbered, not manually numbered
//  - Doesn't yet support OTG, Class or Vendor specific descriptors
//  - Doesn't create a device or device query descriptor, these would be build similar
//    to the way an Endpoint does
//
// The Configuration Descriptor is actually sent back to the host with all the attached
// Interface and Endpoint descriptors, which is why they are packaged up into a single
// buffer. This is the most complex to build.

// https://www.beyondlogic.org/usbnutshell/usb1.shtml
// https://www.usbmadesimple.co.uk/index.html


#include <cstdint>
#include <span>
#include <algorithm>
#include <numeric>
#include <tuple>

namespace usb::descriptor {

    namespace impl {

        // helper to write a uint16_t as little endian ordering into a buffer
        constexpr void write_le(std::span<std::uint8_t, 2> buffer, std::uint16_t data)
        {
            buffer[0] = static_cast<std::uint8_t>(data & 0xFF);
            buffer[1] = static_cast<std::uint8_t>((data >> 8) & 0xFF);
        }

    }

    enum class DescriptorType : std::uint8_t
    {
        Device = 0x01,
        Configuration = 0x02,
        String = 0x03,
        Interface = 0x04,
        EndPoint = 0x05,
        DeviceQualifier = 0x06
    };

    enum class EndpointDirection : std::uint8_t
    {
        Out = 0b0000'0000,  // Host to Device
        In  = 0b1000'0000   // Device to Host
    };

    enum class EndpointTransfer : std::uint8_t
    {
        //              ---- --XX
        Control     = 0b0000'0000,
        Isochronous = 0b0000'0001,
        Bulk        = 0b0000'0010,
        Interrupt   = 0b0000'0011
    };

    enum class EndpointSynchronisation : std::uint8_t
    {
        //               ---- XX--
        None         = 0b0000'0000,
        Asynchronous = 0b0000'0100,
        Adaptive     = 0b0000'1000,
        Synchronous  = 0b0000'1100,
    };

    enum class EndpointUsage : std::uint8_t
    {
        //                   --XX ----
        Data             = 0b0000'0000,
        Feedback         = 0b0001'0000,
        ImplicitFeedback = 0b0010'0000
    };

    //
    // Endpoint allowing full control of definition.
    //
    class Endpoint
    {
    public:
        constexpr static std::size_t DescriptorLength = 7;

        // Endpoint is a literal type, no need to have a builder
        constexpr Endpoint(
            EndpointDirection direction,
            std::uint8_t address,
            EndpointTransfer transfer,
            EndpointSynchronisation synchronisation,
            EndpointUsage usage,
            std::uint16_t maxPacketSize,
            std::uint8_t interval
        )
            : m_Direction(direction)
            , m_Address(address & 0x0F) // max endpoint = 15
            , m_Transfer(transfer)
            , m_Synchronisation(synchronisation)
            , m_Usage(usage)
            , m_MaxPacketSize(maxPacketSize)
            , m_Interval(interval)
        {
        }

        constexpr std::size_t length() const { return DescriptorLength; }

        constexpr void Render(std::span<std::uint8_t> buffer) const
        {
            // Length & descriptor type
            buffer[0] = DescriptorLength;
            buffer[1] = static_cast<std::uint8_t>(DescriptorType::EndPoint);

            buffer[2] = static_cast<std::uint8_t>(m_Direction)
                        | m_Address;

            buffer[3] = static_cast<std::uint8_t>(m_Transfer)
                        | static_cast<std::uint8_t>(m_Synchronisation)
                        | static_cast<std::uint8_t>(m_Usage);

            // max packet size. Little Endian
            impl::write_le(buffer.subspan<4, 2>(), m_MaxPacketSize);

            // interval
            buffer[6] = m_Interval;
        }


    private:
        EndpointDirection m_Direction;
        std::uint8_t m_Address;
        EndpointTransfer m_Transfer;
        EndpointSynchronisation m_Synchronisation;
        EndpointUsage m_Usage;
        std::uint16_t m_MaxPacketSize;
        std::uint8_t m_Interval;
    };

    //
    // Simple definition for bulk endpoint
    //
    class BulkEndpoint : public Endpoint
    {
    public:
        constexpr BulkEndpoint(
            EndpointDirection direction,
            uint8_t address,
            uint16_t maxPacketSize
        ) : Endpoint(
               direction,
               address,
               EndpointTransfer::Bulk,
               EndpointSynchronisation::None,
               EndpointUsage::Data,
               maxPacketSize,
               0
            )
        {}
    };

    //
    // Simple definition for Interrupt endpoint
    //
    class InterruptEndpoint : public Endpoint
    {
    public:
        constexpr InterruptEndpoint(
                EndpointDirection direction,
                std::uint8_t address,
                std::uint16_t maxPacketSize,
                std::uint8_t interval
        ) : Endpoint(
                direction,
                address,
                EndpointTransfer::Interrupt,
                EndpointSynchronisation::None,
                EndpointUsage::Data,
                maxPacketSize,
                interval
        )
        {}
    };


    //
    // A generic interface definition templated on the number of endpoints
    // Use helper function define_interface() and other variants below for
    // ease of use.
    //
    template<std::size_t NumEndpoints>
    class Interface
    {
        // allow our make helper to construct us
        template<typename ... TEPs> friend
        constexpr auto define_interface(
                std::uint8_t interfaceClass,
                std::uint8_t interfaceSubClass,
                std::uint8_t interfaceProtocol,
                std::uint8_t stringIdentifier,
                TEPs ... endpoints);
    public:
        constexpr static std::size_t InterfaceDescriptorLength = 9;


        // compute the length needed to store this interface descriptor and all its endpoints
        constexpr std::size_t length() const {
            const auto epLength = std::accumulate(
                    m_Endpoints.begin(), m_Endpoints.end(), 0,
                    [](auto sum, auto ep) { return sum + ep.length(); });

            return epLength + InterfaceDescriptorLength;
        }

        constexpr void Render(std::span<std::uint8_t> buffer, std::uint8_t interfaceNumber) const {
            buffer[0] = InterfaceDescriptorLength;
            buffer[1] = static_cast<std::uint8_t>(DescriptorType::Interface);
            buffer[2] = interfaceNumber;
            buffer[3] = 0;  // alternateSetting, not supported yet
            buffer[4] = m_Endpoints.size();
            buffer[5] = m_InterfaceClass;
            buffer[6] = m_InterfaceSubClass;
            buffer[7] = m_InterfaceProtocol;
            buffer[8] = m_StringIdentifier;

            std::size_t location{9};
            for (auto &ep: m_Endpoints) {
                const auto len = ep.length();
                ep.Render(buffer.subspan(location, len));
                location += len;
            }
        }

    private:
        constexpr Interface(
                std::uint8_t interfaceClass,
                std::uint8_t interfaceSubClass,
                std::uint8_t interfaceProtocol,
                std::uint8_t stringIdentifier,
                std::array<Endpoint, NumEndpoints> endpoints
        )
                : m_InterfaceClass{interfaceClass}
                , m_InterfaceSubClass{interfaceSubClass}
                , m_InterfaceProtocol{interfaceProtocol}
                , m_StringIdentifier{stringIdentifier}
                , m_Endpoints{endpoints}
        {
        }


        std::uint8_t m_InterfaceClass;
        std::uint8_t m_InterfaceSubClass;
        std::uint8_t m_InterfaceProtocol;
        std::uint8_t m_StringIdentifier;
        std::array<Endpoint, NumEndpoints> m_Endpoints;
    };

    // Helper to define an interface with a less painful calling convention
    template<typename ... TEPs>
    constexpr auto define_interface(
        std::uint8_t interfaceClass,
        std::uint8_t interfaceSubClass,
        std::uint8_t interfaceProtocol,
        std::uint8_t stringIdentifier,
        TEPs ... endpoints)
    {
       return Interface<sizeof...(TEPs)>{
           interfaceClass, interfaceSubClass, interfaceProtocol, stringIdentifier,
           std::array<Endpoint, sizeof...(TEPs)>{ endpoints... }
       };
    }

    template<typename ... TEPs>
    constexpr auto define_vendor_specific_interface(
        std::uint8_t stringIdentifier,
        TEPs ... endpoints)
    {
        return define_interface(0xFF, 0xFF, 0xFF, stringIdentifier, endpoints...);
    }

    //
    // The configuation
    //
    template<std::size_t ... Sizes>
    class Configuration
    {
    public:
        constexpr static std::size_t ConfigurationDescriptorSize = 9;
        constexpr static std::uint8_t NumInterfaces = sizeof...(Sizes);

        constexpr Configuration(
            std::uint8_t  configurationNumber,
            std::uint8_t stringIdentifier,
            bool selfPowered,
            bool remoteWakeup,
            std::uint8_t maxPower_2mAUnits,
            Interface<Sizes>... interfaces
        )
            : m_ConfigurationNumber{configurationNumber}
            , m_StringIdentifier{stringIdentifier}
            , m_SelfPowered{selfPowered}
            , m_RemoteWakeup{remoteWakeup}
            , m_MaxPower_2mAUnits(maxPower_2mAUnits)
            , m_Interfaces{ interfaces... }
        {}


        // compute the length needed to store this interface descriptor and all its endpoints
        constexpr std::size_t length() const {
            return ConfigurationDescriptorSize + std::apply(
                [](auto && ... interfaces)
                {
                    return (0 + ... + interfaces.length());
                },
                m_Interfaces
            );
        }

        constexpr void Render(std::span<std::uint8_t> buffer) const
        {
            buffer[0] = ConfigurationDescriptorSize;
            buffer[1] = static_cast<std::uint8_t>(DescriptorType::Configuration);
            // total length inserted below after rendering content
            buffer[4] = NumInterfaces;
            buffer[5] = m_ConfigurationNumber;
            buffer[6] = m_StringIdentifier;
            buffer[7] =                     0b1000'0000
                        | (m_SelfPowered  ? 0b0100'0000 : 0)
                        | (m_RemoteWakeup ? 0b0010'0000 : 0);
            buffer[8] = m_MaxPower_2mAUnits;

            // track location so we can use it to set total length value in buffer[2-3]
            std::size_t location{ConfigurationDescriptorSize};
            std::apply(
                [&](auto && ... interfaces)
                {
                    std::size_t index{0};

                    auto render = [&](auto i) {
                        auto len = i.length();
                        i.Render(buffer.subspan(location, len), index);
                        location += len;
                    };

                    ((render(interfaces)), ...);
                },
                m_Interfaces
            );

            // total length
            impl::write_le(buffer.subspan<2, 2>(), location);
        }

    private:
        std::uint8_t m_ConfigurationNumber;
        std::uint8_t m_StringIdentifier;
        bool m_SelfPowered;
        bool m_RemoteWakeup;
        std::uint8_t m_MaxPower_2mAUnits;

        std::tuple<Interface<Sizes>...> m_Interfaces;
    };

    // Concept to enforce a lambda
    template<typename T, std::size_t ... Sizes>
    concept CallableGivesConfiguration = requires(T t)
    {
        t();            // is callable
        // result is a Configuration<...>
        std::is_same_v<decltype(t()), Configuration<Sizes...>>;
    };

    //
    // Rendering helper, pass in the lambda to generate the configuration
    //
    template<std::size_t ... Sizes>
    constexpr static auto MakeConfigurationDescriptor(CallableGivesConfiguration<Sizes...> auto makeConfigLambda)
    {
        // build the configuration using the supplied lambda
        constexpr auto cfg = makeConfigLambda();
        constexpr auto len = cfg.length();

        std::array<std::uint8_t, len> data;

        cfg.Render(data);

        return data;
    }

}


#endif // USB_USBDESCRIPTOR_H
