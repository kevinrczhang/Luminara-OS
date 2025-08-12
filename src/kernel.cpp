#include "am79c973.h"
#include "arp.h"
#include "ethernet_frame.h"
#include "driver_manager.h"
#include "gdt.h"
#include "globals.h"
#include "interrupts.h"
#include "keyboard.h"
#include "memory_manager.h"
#include "mouse.h"
#include "pci.h"
#include "task_scheduler.h"
#include "terminal.h"
#include "types.h"

volatile bool tasks_should_stop = false;

void task_yield() {
    __asm__ volatile("hlt");
}

// let's just treat sleep_delay as a delay for now
void sleep_delay(uint16_t cycles) {
    for (volatile uint16_t i = 0; i < cycles; ++i) {
        if (tasks_should_stop) {
            break;
        }
        for (volatile int j = 0; j < 100; ++j);
    }  
}


void task_doggo()
{
    while(!tasks_should_stop) {
        printf("hello! ok heading out now...");
        sleep_delay(100000); 
    }
    printf("Doggo task stopped.\n");
}

void task_donko()
{
    while(!tasks_should_stop) {
        printf("Just woke up, heading out now...");
        sleep_delay(100000); 
    }
    printf("Donko task stopped.\n");
}

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void call_constructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; ++i) {
        (*i)();
    }
}

void print_donkey_ascii()
{
    printf_colored("############### Luminara OS ###############\n", VGA_COLOR_YELLOW_ON_BLACK);
    
    // Donko
    printf_colored("       ___,A.A_  __\n", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
    printf_colored("       \\   ,   7\"_/\n", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
    printf_colored("        ~\"T(  O O)   <- Hee-haw!\n", VGA_COLOR_YELLOW_ON_BLACK);
    printf_colored("          | \\    Y\n", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
    printf_colored("          |  ~\\ .|\n", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
    printf_colored("          |   |`-'\n", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
    printf_colored("          |   |\n", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
    printf_colored("          j   l\n", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
    printf_colored("         /     \\\n", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
    printf_colored("        Y       Y\n", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
    printf_colored("        |_|   |_|   Ready to boot!\n", VGA_COLOR_CYAN_ON_BLACK);
    printf_colored("########################################\n", VGA_COLOR_YELLOW_ON_BLACK);
}

extern "C" void kernel_main(const void* multiboot_structure, uint32_t multiboot_magic_number)
{
    initialize_terminal();
    print_donkey_ascii();
    printf_colored("=== Luminara OS Kernel ===\n", VGA_COLOR_YELLOW_ON_BLACK);
    
    /**
     * Remember when we pushed the multiboot magic number onto the stack in the loader.s code?
     * 
     * We can now pop it off the stack to verify that the kernel was loaded correctly!
     */
    if (multiboot_magic_number == 0x2badb002) {
        printf_colored("✓ Kernel loaded successfully!\n", VGA_COLOR_GREEN_ON_BLACK);
    }  else {
        printf_colored("✗ ERROR: Invalid multiboot magic number!\n", VGA_COLOR_RED_ON_BLACK);
        printf("Expected: 0x2badb002, Got: 0x");
        
        char hex_digits[] { "0123456789ABCDEF" };

        for (int i = 7; i >= 0; --i) {
            put_char(hex_digits[(multiboot_magic_number >> (i * 4)) & 0xF]);
        }

        printf("\n");
    }

    printf("Initializing system components...\n");

    printf("• Setting up GDT... ");
    GlobalDescriptorTable gdt;
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);

    uint32_t* upper_memory = (uint32_t*) (((size_t) multiboot_structure) + 8);
    size_t heap = 10 * 1024 * 1024;
    MemoryManager memory_manager(heap, ((*upper_memory) * 1024) - heap - (10 * 1024));
    
    printf("heap: 0x");
    printf_hex16((heap >> 16) & 0xFFFF);
    printf_hex16((heap      ) & 0xFFFF);
    
    void* allocated = memory_manager.malloc(1024);
    printf("\nallocated: 0x");
    printf_hex16(((size_t)allocated >> 16) & 0xFFFF);
    printf_hex16(((size_t)allocated      ) & 0xFFFF);
    printf("\n");

    printf("• Setting up Task Scheduler... ");
    TaskScheduler task_scheduler;
    // Task task1(&gdt, task_doggo);
    // Task task2(&gdt, task_donko);
    // Remove problematic third task for now
    // task_scheduler.add_task(&task1);
    // task_scheduler.add_task(&task2);
    // task_scheduler.add_task(&task3);

    // printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
    
    printf("• Setting up interrupts... ");
    InterruptManager interrupt_manager(0x20, &gdt, &task_scheduler);
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
    
    printf("• Setting up driver manager... ");
    DriverManager driver_manager;
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
    
    printf("• Registering keyboard driver... ");
    KeyboardDriver keyboard(&interrupt_manager);
    driver_manager.register_driver(&keyboard);
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);

    // Uncomment when you want to enable mouse support
    // printf("• Registering mouse driver... ");
    // MouseDriver mouse(&interrupt_manager);
    // driver_manager.register_driver(&mouse);
    // printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);

    PeripheralComponentInterconnectController PCIController;
    PCIController.select_drivers(&driver_manager, &interrupt_manager);
    
    printf("• Initializing all drivers... ");
    driver_manager.initialize_all_drivers();
    
    printf("• Activating all drivers... ");
    driver_manager.activate_all_drivers();
    
    printf("• Activating interrupts... ");
    interrupt_manager.activate();
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);

    for (int i = 0; i < 10000000; i++) {
        i++;
    }

    uint8_t num_drivers = driver_manager.get_driver_count();
    Am79C973* am79c973 = (Am79C973*) driver_manager.get_driver(num_drivers - 1);

    auto make_ip = [](uint8_t a, uint8_t b, uint8_t c, uint8_t d) -> uint32_t {
        return ((uint32_t)d << 24) | ((uint32_t)c << 16) | ((uint32_t)b << 8) | (uint32_t)a;
    };

    uint32_t device_ip = make_ip(10, 0, 2, 15);   // 10.0.2.15
    uint32_t gateway_ip = make_ip(10, 0, 2, 2);   // 10.0.2.2

    am79c973->set_ip_address(device_ip);

    EthernetFrameProvider ethernet_frame(am79c973);

    AddressResolutionProtocol arp(&ethernet_frame);
    arp.resolve(gateway_ip);

    // am79c973->send((uint8_t*) "Hello World", 11);
    
    printf(am79c973->get_driver_name());
    
    // printf("\n");
    // printf_colored("System initialized successfully!\n", VGA_COLOR_GREEN_ON_BLACK);
    // printf_colored("Tasks are running. Press ESC to stop them.\n", VGA_COLOR_YELLOW_ON_BLACK);
    // printf_colored("Function keys (F1-F5) show debug messages.\n", VGA_COLOR_CYAN_ON_BLACK);
    
    while(1) {
        // To save power, we can halt until we receive the next interrupt.
        __asm__ volatile("hlt");
    }
}
