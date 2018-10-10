# Sitewhere-Linux
> C library that simplify sending data to SiteWhere from any Linux based system.

C library that allows Linux system to interact with SiteWhere. You can register a device with the SiteWhere, publish events using the provided API and receive commands which can be used to execute code on the device. Messages like SendMeasurement, SendLocation, SendAlert are implemented also abnormal termination are handled. The protocol(MQTT) is used to create a persistent connection between the Linux-application and a SiteWhere instance.

## Developer Setup
- Linux platform
- gnome-terminal
- Internet connection

## Quickstart
1. Clone this repository.
2. Install some dependencies
    - Download MQTTClient-C library from [eclipse paho](https://www.eclipse.org/paho/downloads.php)
    - Traverse till that downloaded directory using $ cd command and enter the following commands
      ```
      $ sudo apt-get install cmake
      $ cmake -DPAHO_WITH_OPENSSL=TRUE
      $ make
      $ sudo make install
      ```
3. Now go to SiteWhere-Linux(cloned repository) directory using $ cd command and enter the below commands
    ```
    $ make
    $ make run
    ```

## Sample Application
The sample application can be found in the SiteWhere-Linux(sw_test.c) directory. The application demostrates how an Linux system can be an IoT gateway and/or client device for SiteWhere. As an IoT gateway you can register an Linux system with SiteWhere and send location, alert, measurement events to SiteWhere. GUI is not there so you need to know some terminal basic.

## Resources
You can refer the SiteWhere Documentation from here:
    - [Introduction](http://sitewhere.io/docs/en/2.0.RC1/platform/index.html)
    - [userguide](http://documentation.sitewhere.io/userguide.html)
