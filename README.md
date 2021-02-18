# WEATHER STATION
Project requires: Raspberry Pi and BME280 sensor, connected via I2C interface. Moreover, account on ThingSpeak cloud system is necessary - user key should be provided in **run_wheather_station.py**.

Build:
> **make clean**
> **make**

Run:
> **sudo python3 run_wheather_station.py**

For automatic start after each platform boot, consider adding below command at the end of **/etc/profile** file:
> **cd /path/to/project**
> **sudo python3 run_wheather_station.py &**
