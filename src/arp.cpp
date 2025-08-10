#include "arp.h"

AddressResolutionProtocol::AddressResolutionProtocol(EthernetFrameProvider* backend)
    :  EthernetFrameHandler(backend, 0x806)
{
    num_cache_entries = 0;
}

AddressResolutionProtocol::~AddressResolutionProtocol()
{

}

bool AddressResolutionProtocol::on_ether_frame_received(uint8_t* payload, uint32_t size)
{
    if (size < sizeof(AddressResolutionProtocolMessage)) {
        return false;
    }
    
    AddressResolutionProtocolMessage* arp = (AddressResolutionProtocolMessage*) payload;
    if (arp->hardware_type == 0x0100) {
        
        if (arp->protocol == 0x0008
        && arp->hardware_address_size == 6
        && arp->protocol_address_size == 4
        && arp->destination_ip == backend->get_ip_address())
        {
            
            switch(arp->command) {
                
                case 0x0100: // request
                    arp->command = 0x0200;
                    arp->destination_ip = arp->source_ip;
                    arp->destination_mac = arp->source_mac;
                    arp->source_ip = backend->get_ip_address();
                    arp->source_mac = backend->get_mac_address();
                    return true;
                    break;
                    
                case 0x0200: // response
                    if (num_cache_entries < 128) {
                        ip_cache[num_cache_entries] = arp->source_ip;
                        mac_cache[num_cache_entries] = arp->source_mac;
                        num_cache_entries++;
                    }
                    break;
            }
        }
        
    }
    
    return false;
}

void AddressResolutionProtocol::request_mac_address(uint32_t ip)
{
    AddressResolutionProtocolMessage arp_message;
    arp_message.hardware_type = 0x0100; // ethernet
    arp_message.protocol = 0x0008; // ipv4
    arp_message.hardware_address_size = 6; // mac
    arp_message.protocol_address_size = 4; // ipv4
    arp_message.command = 0x0100; // request
    arp_message.source_mac = backend->get_mac_address();
    arp_message.source_ip = backend->get_ip_address();
    arp_message.destination_mac = 0xFFFFFFFFFFFF; // broadcast
    arp_message.destination_ip = ip;

    this->send(arp_message.destination_mac, (uint8_t*) &arp_message, sizeof(AddressResolutionProtocolMessage));
}

uint64_t AddressResolutionProtocol::get_mac_from_cache(uint32_t ip)
{
    for (int i = 0; i < num_cache_entries; ++i) {
        if (ip_cache[i] == ip) {
            return mac_cache[i];
        }
    }
    return 0xFFFFFFFFFFFF; // broadcast address
}

uint64_t AddressResolutionProtocol::resolve(uint32_t ip)
{
    uint64_t result = get_mac_from_cache(ip);

    if (result == 0xFFFFFFFFFFFF) {
        request_mac_address(ip);
    }

    while (result == 0xFFFFFFFFFFFF) {
        // possible infinite loop
        result = get_mac_from_cache(ip);
    }
    
    return result;
}
