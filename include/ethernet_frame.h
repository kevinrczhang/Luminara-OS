#ifndef ETHERNET_FRAME_H
#define ETHERNET_FRAME_H

#include "am79c973.h"
#include "types.h"

// NOTE: All of these values are in big endian
struct EthernetFrameHeader
{
    uint64_t destination_mac : 48;
    uint64_t source_mac : 48;
    uint16_t ether_type;
} __attribute__((packed));

typedef uint32_t EthernetFrameFooter;

class EthernetFrameProvider;

class EthernetFrameHandler
{
protected:
    EthernetFrameProvider* backend;
    uint16_t ether_type;
        
public:
    EthernetFrameHandler(EthernetFrameProvider* backend, uint16_t ether_type);
    ~EthernetFrameHandler();
    
    virtual bool on_ethernet_frame_received(uint8_t* payload, uint32_t size);
    void send(uint64_t destination_mac, uint8_t* payload, uint32_t size);
    
};
        
        
class EthernetFrameProvider : public RawDataHandler
{
friend class EthernetFrameHandler;
protected:
    EthernetFrameHandler* handlers[65535];
public:
    EthernetFrameProvider(Am79C973* backend);
    ~EthernetFrameProvider();
    
    bool on_raw_data_received(uint8_t* buffer, uint32_t size);
    void send(uint64_t destination_mac, uint16_t ether_type, uint8_t* buffer, uint32_t size);
    uint64_t get_mac_address();
    uint32_t get_ip_address();
};

#endif
