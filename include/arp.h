#ifndef ARP_H
#define ARP_H

#include "am79c973.h"
#include "ethernet_frame.h"
#include "types.h"

struct AddressResolutionProtocolMessage
{
    uint16_t hardware_type;
    uint16_t protocol;
    uint8_t hardware_address_size; // 6
    uint8_t protocol_address_size; // 4
    uint16_t command;
    

    uint64_t source_mac : 48;
    uint32_t source_ip;
    uint64_t destination_mac : 48;
    uint32_t destination_ip;
    
} __attribute__((packed));

class AddressResolutionProtocol : EthernetFrameHandler
{
    uint32_t ip_cache[128];
    uint64_t mac_cache[128];
    int num_cache_entries;

    public:
        AddressResolutionProtocol(EthernetFrameProvider* backend);
        ~AddressResolutionProtocol();

        bool on_ether_frame_received(uint8_t* payload, uint32_t size);

        void request_mac_address(uint32_t ip);
        uint64_t get_mac_from_cache(uint32_t ip);
        uint64_t resolve(uint32_t ip);
};

#endif
