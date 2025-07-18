#ifndef DRIVER_MANAGER_H
#define DRIVER_MANAGER_H

#include "types.h"
#include "driver.h"

// Maximum number of drivers that can be managed
#define MAX_DRIVERS 16

class DriverManager
{
private:
    Driver* drivers[MAX_DRIVERS];
    uint8_t driver_count;
    bool is_active;

    int find_driver_index(Driver* driver);

public:
    DriverManager();
    ~DriverManager();

    // this basically just adds the driver to the table we have above
    bool register_driver(Driver* driver);
    // this just removes it (obviously duh)
    bool unregister_driver(Driver* driver);
    
    // Driver lifecycle management
    void initialize_all_drivers();
    void activate_all_drivers();
    void deactivate_all_drivers();
    void reset_all_drivers();
    
    // Driver information and status
    uint8_t get_driver_count() const;
    Driver* get_driver(uint8_t index);
    Driver* find_driver(const char* name);
    bool is_driver_registered(const char* name);
    
    // System status
    bool is_manager_active() const;
    void set_active(bool active);
    
    // Debugging and information
    void print_driver_status();
    
    // Cleanup
    void shutdown_all_drivers();
};

#endif
