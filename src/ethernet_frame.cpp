#include "ethernet_frame.h"

EthernetFrameHandler::EthernetFrameHandler(EthernetFrameProvider* backend, uint16_t ether_type)
{
    this->ether_type = ((ether_type & 0x00FF) << 8) | ((ether_type & 0xFF00) >> 8);
    this->backend = backend;
    backend->handlers[ether_type] = this;
}

EthernetFrameHandler::~EthernetFrameHandler()
{
    backend->handlers[ether_type] = 0;
}

bool EthernetFrameHandler::on_ethernet_frame_received(uint8_t* payload, uint32_t size)
{
    return false;
}

void EthernetFrameHandler::send(uint64_t destination_mac, uint8_t* data, uint32_t size)
{
    backend->send(destination_mac, ether_type, data, size);
}
EthernetFrameProvider::EthernetFrameProvider(Am79C973* backend)
    : RawDataHandler(backend)
{
    for (uint32_t i = 0; i < 65535; i++) { // I guess we couild prob use a 16 bit int here
        handlers[i] = 0;
    }
}

EthernetFrameProvider::~EthernetFrameProvider()
{

}

bool EthernetFrameProvider::on_raw_data_received(uint8_t* buffer, uint32_t size)
{
    EthernetFrameHeader* frame { (EthernetFrameHeader*) buffer };
    bool send_back { false };

    if (frame->destination_mac == 0xFFFFFFFFFFFF || frame->destination_mac == backend->get_mac_address()) {
        if (handlers[frame->ether_type] != 0) {
            send_back = handlers[frame->ether_type]->on_ethernet_frame_received(buffer + sizeof(EthernetFrameHeader), size - sizeof(EthernetFrameHeader));
        }
    }

    if (send_back) {
        frame->destination_mac = frame->source_mac;
        frame->source_mac = backend->get_mac_address();
    }

    return send_back;
}

void EthernetFrameProvider::send(uint64_t destination_mac, uint16_t ether_type, uint8_t* buffer, uint32_t size)
{
    uint8_t* buffer2 { (uint8_t*) MemoryManager::memory_manager->malloc(sizeof(EthernetFrameHeader) + size) };
    EthernetFrameHeader* frame { (EthernetFrameHeader*) buffer2 };

    frame->destination_mac = destination_mac;
    frame->source_mac = backend->get_mac_address();
    frame->ether_type = ether_type;

    uint8_t* source { buffer };
    uint8_t* destination { buffer2 + sizeof(EthernetFrameHeader) };

    for (uint32_t i = 0; i < size; ++i) {
        destination[i] = source[i];
    }

    backend->send(buffer2, size + sizeof(EthernetFrameHeader));
}

uint64_t EthernetFrameProvider::get_mac_address() {
    return backend->get_mac_address();
}

uint32_t EthernetFrameProvider::get_ip_address() {
    return backend->get_ip_address();
}
