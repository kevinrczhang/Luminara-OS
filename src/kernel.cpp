#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "mouse.h"
#include "pci.h"
#include "task_scheduler.h"
#include "terminal.h"
#include "types.h"
#include "driver_manager.h"

void task_doggo()
{
    while(true)
        printf("Doggo");
}
void task_donko()
{
    while(true)
        printf("Donko");
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
    printf_colored("############### DONKEY OS ###############\n", VGA_COLOR_YELLOW_ON_BLACK);
    
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
    printf_colored("=== Donkey OS Kernel ===\n", VGA_COLOR_YELLOW_ON_BLACK);
    
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

    printf("• Setting up Task Scheduler... ");
    TaskScheduler task_scheduler;
    Task task1(&gdt, task_doggo);
    Task task2(&gdt, task_donko);
    task_scheduler.add_task(&task1);
    task_scheduler.add_task(&task2);
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
    
    printf("• Setting up interrupts... ");
    InterruptManager interrupt_manager(0x20, &gdt, &task_scheduler);
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
    
    printf("• Setting up driver manager... ");
    DriverManager driver_manager;
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
    
    // printf("• Registering keyboard driver... ");
    // KeyboardDriver keyboard(&interrupt_manager);
    // driver_manager.register_driver(&keyboard);
    // printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);

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
    
    // printf("\n");
    // printf_colored("System initialized successfully!\n", VGA_COLOR_GREEN_ON_BLACK);
    // driver_manager.print_driver_status();
    // printf("Hardware cursor is visible and responsive.\n");
    // printf("Type anything - use backspace to delete.\n");
    // printf("Function keys (F1-F5) show debug messages and utilities.\n");
    // printf("Press F5 to clear screen. Have fun!\n");
    // printf("Hardware cursor is visible and responsive.\n");
    // printf("Type anything - use backspace to delete.\n");
    // printf("Function keys (F1-F5) show debug messages and utilities.\n");
    // printf("Press F5 to clear screen. Have fun!\n");

    while(1) {
        // To save power, we can halt until we receive the next interrupt.
        __asm__ volatile("hlt");
    }
}
