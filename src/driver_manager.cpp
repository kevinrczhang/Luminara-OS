#include "driver_manager.h"
#include "terminal.h"

DriverManager::DriverManager()
    : driver_count(0), is_active(false)
{
    for (int i = 0; i < MAX_DRIVERS; ++i) {
        drivers[i] = nullptr;
    }
}

DriverManager::~DriverManager()
{
    shutdown_all_drivers();
}

int DriverManager::find_driver_index(Driver* driver)
{
    if (driver == nullptr) {
        return -1;
    }
    
    for (int i = 0; i < driver_count; ++i) {
        if (drivers[i] == driver) {
            return i;
        }
    }
    
    return -1;
}


bool DriverManager::register_driver(Driver* driver)
{
    if (driver == nullptr) {
        printf_colored("ERROR: Cannot register null driver\n", VGA_COLOR_RED_ON_BLACK);
        return false;
    }
    
    if (driver_count >= MAX_DRIVERS) {
        printf_colored("ERROR: Maximum number of drivers reached\n", VGA_COLOR_RED_ON_BLACK);
        return false;
    }
    
    if (find_driver_index(driver) != -1) {
        printf_colored("WARNING: Driver already registered\n", VGA_COLOR_YELLOW_ON_BLACK);
        return false;
    }
    
    // Register the driver
    drivers[driver_count] = driver;
    driver_count++;
    
    printf("Driver registered: ");
    printf_colored(driver->get_driver_name(), VGA_COLOR_CYAN_ON_BLACK);
    printf("\n");
    
    return true;
}

bool DriverManager::unregister_driver(Driver* driver)
{
    int index { find_driver_index(driver) };
    if (index == -1) {
        printf_colored("ERROR: Driver not found for unregistration\n", VGA_COLOR_RED_ON_BLACK);
        return false;
    }
    
    printf("Unregistering driver: ");
    printf_colored(drivers[index]->get_driver_name(), VGA_COLOR_CYAN_ON_BLACK);
    printf("\n");
    
    // Deactivate driver before unregistering
    drivers[index]->deactivate();
    
    // Shift remaining drivers down
    for (int i = index; i < driver_count - 1; i++) {
        drivers[i] = drivers[i + 1];
    }
    
    driver_count--;
    drivers[driver_count] = nullptr;  // Clear the last slot
    
    return true;
}

// iterate through all drivers and initialize them
void DriverManager::initialize_all_drivers()
{
    printf("Initializing all drivers...\n");
    
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i] != nullptr) {
            printf("• Initializing ");
            printf_colored(drivers[i]->get_driver_name(), VGA_COLOR_CYAN_ON_BLACK);
            printf("... ");
            
            drivers[i]->initialize();
            printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
        }
    }
}

void DriverManager::activate_all_drivers()
{
    printf("Activating all drivers...\n");
    
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i] != nullptr) {
            printf("• Activating ");
            printf_colored(drivers[i]->get_driver_name(), VGA_COLOR_CYAN_ON_BLACK);
            printf("... ");
            
            drivers[i]->activate();
            printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
        }
    }
    
    is_active = true;
}

void DriverManager::deactivate_all_drivers()
{
    printf("Deactivating all drivers...\n");
    
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i] != nullptr) {
            printf("• Deactivating ");
            printf_colored(drivers[i]->get_driver_name(), VGA_COLOR_CYAN_ON_BLACK);
            printf("... ");
            
            drivers[i]->deactivate();
            printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
        }
    }
    
    is_active = false;
}

void DriverManager::reset_all_drivers()
{
    printf("Resetting all drivers...\n");
    
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i] != nullptr) {
            printf("• Resetting ");
            printf_colored(drivers[i]->get_driver_name(), VGA_COLOR_CYAN_ON_BLACK);
            printf("... ");
            
            drivers[i]->reset();
            printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
        }
    }
}

uint8_t DriverManager::get_driver_count() const
{
    return driver_count;
}

Driver* DriverManager::get_driver(uint8_t index)
{
    if (index >= driver_count) {
        return nullptr;
    }
    
    return drivers[index];
}

bool DriverManager::is_manager_active() const
{
    return is_active;
}

void DriverManager::set_active(bool active)
{
    is_active = active;
}

void DriverManager::print_driver_status()
{
    printf_colored("=== Driver Manager Status ===\n", VGA_COLOR_YELLOW_ON_BLACK);
    printf("Total drivers: ");
    put_char_colored((char)(driver_count + '0'), VGA_COLOR_WHITE_ON_BLACK);
    printf("/");
    put_char_colored((char)(MAX_DRIVERS + '0'), VGA_COLOR_WHITE_ON_BLACK);
    printf("\n");
    
    printf("Manager status: ");
    if (is_active) {
        printf_colored("ACTIVE\n", VGA_COLOR_GREEN_ON_BLACK);
    } else {
        printf_colored("INACTIVE\n", VGA_COLOR_RED_ON_BLACK);
    }
    
    printf_colored("============================\n", VGA_COLOR_YELLOW_ON_BLACK);
}

void DriverManager::shutdown_all_drivers()
{
    printf("Shutting down all drivers...\n");
    
    // Deactivate and unregister all drivers in reverse order
    for (int i = driver_count - 1; i >= 0; i--) {
        if (drivers[i] != nullptr) {
            printf("• Shutting down ");
            printf_colored(drivers[i]->get_driver_name(), VGA_COLOR_CYAN_ON_BLACK);
            printf("... ");
            
            drivers[i]->deactivate();
            drivers[i] = nullptr;
            printf_colored("OK\n", VGA_COLOR_GREEN_ON_BLACK);
        }
    }
    
    driver_count = 0;
    is_active = false;
    
    printf_colored("All drivers shut down.\n", VGA_COLOR_GREEN_ON_BLACK);
}
