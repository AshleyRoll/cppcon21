#ifndef UTILS_USBDESCRIPTOR_H
#define UTILS_USBDESCRIPTOR_H


// https://www.beyondlogic.org/usbnutshell/usb1.shtml
// https://www.usbmadesimple.co.uk/index.html


#include <cstdint>
#include <span>

namespace utils::usb {

    namespace impl {

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
    // Endpoint allowing full control of definition. Suggested for Isochronous Endpoints
    // or strange configurations
    //
    class Endpoint
    {
    public:
        constexpr static std::size_t DescriptorLength = 7;

        constexpr Endpoint(
            EndpointDirection direction,
            uint8_t address,
            EndpointTransfer transfer,
            EndpointSynchronisation synchronisation,
            EndpointUsage usage,
            uint16_t maxPacketSize,
            uint8_t interval
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

        constexpr void Render(std::span<std::uint8_t, DescriptorLength> buffer) const
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
        uint8_t m_Address;
        EndpointTransfer m_Transfer;
        EndpointSynchronisation m_Synchronisation;
        EndpointUsage m_Usage;
        uint16_t m_MaxPacketSize;
        uint8_t m_Interval;
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


}


#endif //UTILS_USBDESCRIPTOR_H
