#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "terminal.h"
#include "types.h"

// This defines a function pointer type for constructors.
typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

// We want to call all the global constructors we defined before main.
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

/**
 * The main kernel entry point
 * 
 * This is where the kernel starts execution after being loaded by the bootloader.
 * It initializes all system components and then enters into an infinite loop.
 */
extern "C" void kernel_main(const void* multiboot_structure, uint32_t multiboot_magic_number)
{
    // We initialize the terminal first so we can display output.
    initialize_terminal();
    
    // Print the donkey ASCII art first! (I eventually want to make the screen bigger).
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
        
        // This is to display the magic number we got in hex.
        char hex_digits[] = "0123456789ABCDEF";

        for (int i = 7; i >= 0; --i) {
            put_char(hex_digits[(multiboot_magic_number >> (i * 4)) & 0xF]);
        }

        printf("\n");
    }

    printf("Initializing system components...\n");

    printf("• Setting up GDT... ");
    GlobalDescriptorTable gdt;
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
    
    printf("• Setting up interrupts... ");
    InterruptManager interrupts(0x20, &gdt);
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
    
    printf("• Setting up keyboard... ");
    KeyboardDriver keyboard(&interrupts);
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
    
    printf("• Activating interrupts... ");
    interrupts.activate();
    printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
    
    printf("\n");
    printf_colored("System initialized successfully!\n", VGA_COLOR_GREEN_ON_BLACK);
    printf("Hardware cursor is visible and responsive.\n");
    printf("Type anything - use backspace to delete.\n");
    printf("Function keys (F1-F5) show debug messages and utilities.\n");
    printf("Press F5 to clear screen. Have fun!\n");

    while(1) {
        // To save power, we can halt until we receive the next interrupt.
        __asm__ volatile("hlt");
    }
}
