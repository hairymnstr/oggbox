#ifndef POWER_H
#define POWER_H 1

struct power_info {
  int battery_voltage;
};

void start_power_management_task();
int power_latest_battery();

#endif /* ifndef POWER_H */
