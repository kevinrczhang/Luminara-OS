#include "interrupts.h"
#include "terminal.h"

extern "C" uint32_t handle_interrupt_wrapper(uint8_t interrupt, uint32_t esp)
{
    return InterruptManager::handle_interrupt(interrupt, esp);
}

InterruptHandler::InterruptHandler(InterruptManager* interrupt_manager, uint8_t interrupt_number)
{
    this->interrupt_number = interrupt_number;
    this->interrupt_manager = interrupt_manager;
    interrupt_manager->handlers[interrupt_number] = this;
}

InterruptHandler::~InterruptHandler()
{
    if (interrupt_manager->handlers[interrupt_number] == this) {
        interrupt_manager->handlers[interrupt_number] = 0;
    }
}

uint32_t InterruptHandler::handle_interrupt(uint32_t esp)
{
    return esp;
}

InterruptManager::GateDescriptor InterruptManager::interrupt_descriptor_table[256];
InterruptManager* InterruptManager::active_interrupt_manager = { 0 };

void InterruptManager::set_interrupt_descriptor_table_entry(uint8_t interrupt,
    uint16_t code_segment, void (*handler)(), uint8_t descriptor_privilege_level, uint8_t descriptor_type)
{
    interrupt_descriptor_table[interrupt].handler_address_low_bits = ((uint32_t) handler) & 0xFFFF;
    interrupt_descriptor_table[interrupt].handler_address_high_bits = (((uint32_t) handler) >> 16) & 0xFFFF;
    interrupt_descriptor_table[interrupt].gdt_code_segment_selector = code_segment;

    const uint8_t IDT_DESC_PRESENT { 0x80 };
    interrupt_descriptor_table[interrupt].access = IDT_DESC_PRESENT | ((descriptor_privilege_level & 3) << 5) | descriptor_type;
    interrupt_descriptor_table[interrupt].reserved = 0;
}

InterruptManager::InterruptManager(uint16_t hardware_interrupt_offset, GlobalDescriptorTable* global_descriptor_table)
    // We initialize the pic ports with their standard addresses. See link: https://wiki.osdev.org/8259_PIC.
    : pic_master_command_port(0x20),
      pic_master_data_port(0x21),
      pic_slave_command_port(0xA0),
      pic_slave_data_port(0xA1)
{
    this->hardware_interrupt_offset_value = hardware_interrupt_offset;
    uint32_t code_segment { global_descriptor_table->get_code_segment_selector() };

    const uint8_t IDT_INTERRUPT_GATE { 0xE };
    
    for (uint8_t i = 255; i > 0; --i) {
        set_interrupt_descriptor_table_entry(i, code_segment, &interrupt_ignore, 0, IDT_INTERRUPT_GATE);
        handlers[i] = 0;
    }

    set_interrupt_descriptor_table_entry(0, code_segment, &interrupt_ignore, 0, IDT_INTERRUPT_GATE);
    handlers[0] = 0;

    set_interrupt_descriptor_table_entry(0x00, code_segment, &handle_exception_0x00, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x01, code_segment, &handle_exception_0x01, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x02, code_segment, &handle_exception_0x02, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x03, code_segment, &handle_exception_0x03, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x04, code_segment, &handle_exception_0x04, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x05, code_segment, &handle_exception_0x05, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x06, code_segment, &handle_exception_0x06, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x07, code_segment, &handle_exception_0x07, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x08, code_segment, &handle_exception_0x08, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x09, code_segment, &handle_exception_0x09, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0A, code_segment, &handle_exception_0x0a, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0B, code_segment, &handle_exception_0x0b, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0C, code_segment, &handle_exception_0x0c, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0D, code_segment, &handle_exception_0x0d, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0E, code_segment, &handle_exception_0x0e, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0F, code_segment, &handle_exception_0x0f, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x10, code_segment, &handle_exception_0x10, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x11, code_segment, &handle_exception_0x11, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x12, code_segment, &handle_exception_0x12, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x13, code_segment, &handle_exception_0x13, 0, IDT_INTERRUPT_GATE);

    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x00, code_segment, &handle_interrupt_request_0x00, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x01, code_segment, &handle_interrupt_request_0x01, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x02, code_segment, &handle_interrupt_request_0x02, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x03, code_segment, &handle_interrupt_request_0x03, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x04, code_segment, &handle_interrupt_request_0x04, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x05, code_segment, &handle_interrupt_request_0x05, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x06, code_segment, &handle_interrupt_request_0x06, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x07, code_segment, &handle_interrupt_request_0x07, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x08, code_segment, &handle_interrupt_request_0x08, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x09, code_segment, &handle_interrupt_request_0x09, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x0A, code_segment, &handle_interrupt_request_0x0a, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x0B, code_segment, &handle_interrupt_request_0x0b, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x0C, code_segment, &handle_interrupt_request_0x0c, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x0D, code_segment, &handle_interrupt_request_0x0d, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x0E, code_segment, &handle_interrupt_request_0x0e, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset_value + 0x0F, code_segment, &handle_interrupt_request_0x0f, 0, IDT_INTERRUPT_GATE);

    pic_master_command_port.write(0x11);  // Initialize both master and slave PICs.
    pic_slave_command_port.write(0x11);

    // Remap IRQ vectors
    pic_master_data_port.write(hardware_interrupt_offset_value);      // Master PIC offset while slave starts at offset + 8.
    pic_slave_data_port.write(hardware_interrupt_offset_value + 8); 
    // Configure PIC cascade
    pic_master_data_port.write(0x04);  // Tell master PIC that slave is at IRQ2.
    pic_slave_data_port.write(0x02);   // Tell slave PIC its cascade identity.

    // Set PIC mode
    pic_master_data_port.write(0x01);  // 8086/88 mode
    pic_slave_data_port.write(0x01);

    // Enable all interrupts (mask = 0x00 means no IRQs are masked).
    pic_master_data_port.write(0x00);
    pic_slave_data_port.write(0x00);

    // Load the IDT
    InterruptDescriptorTablePointer idt_pointer;
    idt_pointer.size = 256 * sizeof(GateDescriptor) - 1;
    idt_pointer.base = (uint32_t)interrupt_descriptor_table;

    // "lidt" is an asm instruction that loads the IDT.
    asm volatile("lidt %0" : : "m" (idt_pointer));
}

InterruptManager::~InterruptManager()
{
    deactivate();
}

uint16_t InterruptManager::get_hardware_interrupt_offset()
{
    return hardware_interrupt_offset_value;
}

void InterruptManager::activate()
{
    if (active_interrupt_manager != 0) {
        active_interrupt_manager->deactivate();
    }

    active_interrupt_manager = this;
    asm("sti");  // Enable interrupts
}

void InterruptManager::deactivate()
{
    if (active_interrupt_manager == this) {
        active_interrupt_manager = 0;
        asm("cli");  // Disable interrupts
    }
}

uint32_t InterruptManager::handle_interrupt(uint8_t interrupt, uint32_t esp)
{
    if (active_interrupt_manager != 0) {
        return active_interrupt_manager->do_handle_interrupt(interrupt, esp);
    }
    return esp;
}

uint32_t InterruptManager::do_handle_interrupt(uint8_t interrupt, uint32_t esp)
{
    if (handlers[interrupt] != 0) {
        esp = handlers[interrupt]->handle_interrupt(esp);
    } else if (interrupt != hardware_interrupt_offset_value) {
        char error_msg[] { "UNHANDLED INTERRUPT 0x00" };
        char hex_digits[] { "0123456789ABCDEF" };
        error_msg[22] = hex_digits[(interrupt >> 4) & 0xF];
        error_msg[23] = hex_digits[interrupt & 0xF];
        printf(error_msg);
    }

    if (hardware_interrupt_offset_value <= interrupt && interrupt < hardware_interrupt_offset_value + 16) {
        pic_master_command_port.write(0x20);  // EOI is always sent to the master PIC.
        if (hardware_interrupt_offset_value + 8 <= interrupt) {
            pic_slave_command_port.write(0x20);  // EOI is sent to slave PIC if the interrupt came from it.
        }
    }

    return esp;
}
