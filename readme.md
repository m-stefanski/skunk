# Skunks

Skunks (Polish for Skunk) is a small air quality meter aimed for use in developing countries, home projects and in every place where affordable air quality meter might be useful. 

It is designed to be cheap and easy to make from over-the-counter parts.

## Capabilities

Skunk can measure following air parameters:

* PM 2,5
* Temperature
* Humidity
* Pressure

Results are available as a website presented when user connects to a WiFi network the device is broadcasting. It is also possible to configure skunk to connect to any available 2.4 GHz WiFi network and view its results there.

## Hardware

* [WeMOS D1 mini](https://www.wemos.cc/en/latest/d1/d1_mini.html) - for CPU / Memory / IO and WiFi capabilities
* [GP2Y1010AU0F](https://www.sharpsde.com/products/optoelectronic-components/model/GP2Y1010AU0F/) - dust sensor from Sharp
* [BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/) - temprerature, humidity and air pressure sensor from BOSH

Data sheets for used components are available in [datasheets](/datasheets) directory.

## Designs

All sources are available in [src](/src) directory. Web content needs to be converted to `.h` files:

```
git clone https://github.com/Jamesits/bin2array.git 
cd ./src/web_content
find . -iname "*.*"  -type f -exec python3 ./../../bin2array/bin2array.py '{}' -O ./../'{}'.h \;
```

BB overview is shown below.

![](docs/prototype_bb_BME280_DSM501.png)

Electrical schematics:

![](docs/prototype_electrical_BME280_DSM501.png)

PCB:

![](docs/prototype_pcb_BME280_DSM501.png)

And very, VERY unholy real photos of firts prototype:

![](docs/prototype_bb_BME280_DSM501_live1.png)
![](docs/prototype_bb_BME280_DSM501_live2.png)

All hardware designs are stored in [board](/board) directory. Old iterations are kept as well, should the hardware change.

Skunk icon is stored in [icon](/icon) directory. It is a skunk glyph from [OpenMoji](https://openmoji.org) with added background. Due to limited space it is exported as 4-bit grayscale 180x180 px.
