
# Datalogger with Blutooth Terminal Readout
This Arduino sketch logs date, time, temperature, humidity and pressure in .csv format on an SD card and also outputs this information to a bluetooth terminal for a live view of the data being written. Upon reset the sketch creates a new .csv file.
## Libraries used
* SoftwareSerial (Arduino built in library)
* SD (Arduino built in library)
* Wire (Arduino built in library)
* [DHT Sensor Library](https://github.com/plasmajedigdp/system_code/tree/master/libraries/DHT_sensor_library)
* [Adafruit_Sensor](https://github.com/plasmajedigdp/system_code/tree/master/libraries/Adafruit_Unified_Sensor)
* [Adafruit_BMP085_U](https://github.com/plasmajedigdp/system_code/tree/master/libraries/Adafruit_BMP085_Unified)
* [DS3231](https://github.com/plasmajedigdp/system_code/tree/master/libraries/DS3231)

If there are any issues feel free to contact contact [MaxSGarcia](https://github.com/MaxSGarcia)
