# FindMeSAT
GPS assisted tracking device with GSM (SMS & GPRS) and Bluetooth connectivity.

## List of devices
* __CPU: Atmel ATxmega256A3BU__ (256kB Flash / 16kB RAM / 4kB EEPROM)
* __GSM / GPS / BT: SIM808__ (SMS / GPRS / ...)
* __VHF: AX5243__ (VHF TX/RX)
* __Accel/Gyro/Mag: MPU-9250__ (3Axis-Acceleration, 3Axis-Gyro, 3Axis-Magneto)
* __Baro/Temp: MS560702BA03-50__ (0.2 meter height resolution, better than 0.01°C resolution)
* __Hygro: SHT31-DIS__ (hygrometer within the air stream of the board)
* __VCTCXO: CFPT-141_20MHz__ (being synchronized to the GPS 1PPS signal by pulling the VC)
* __GSM antenna: within the PCB__
* __BT antenna: within the PCB__
* __GPS antenna: external active antenna__ via MCX connector
* __VHF antenna: external passive antenna__ via SMA connector
* __LiPo: 1-cell 3.7V__ (high power request by the SIM808 device to be satisfied)

###Current PCB overview:
![current PCB overview](https://raw.githubusercontent.com/DF4IAH/FindMeSAT/master/Docs/64_Redesign_1V3/Placement/2_Eagle-Layout_b.png)
