#ifndef POWER_H
#define POWER_H 1

struct power_info {
  int battery_voltage;
};

void start_power_management_task();
void power_shutdown();
void power_sleep();
int power_latest_battery();
void power_set_charge_full();
void power_set_charge_slow();
void power_set_charge_none();
int power_get_usb_present();
void power_aux_on(void);

#endif /* ifndef POWER_H */
