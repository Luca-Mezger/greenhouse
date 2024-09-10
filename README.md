# Automatic Greenhouse

The goal for this project week was to make a fully functioning automated greenhouse, where all funtions that would need manual work, would be able to be done by the greenhouse.

## Arduino

We used Arduino a hardware and software platform. Arduino has these mini computers. Where not a lot of space is needed and it is possible to code what they do.


## Todo
- [ ] Detection Water Supply (empty or not)
- [ ] Temperature Sensor/Fan
- [ ] Build Greenhouse
- [ ] Display
- [ ] Smartphone Connection
- [ ] Poster
- [ ] Report



## Work journal
### 09 September 2024

#### Decided on Materials needed:
  - Arduino Nano 33 Sense BLE (contains various sensors)
  - Soil Moisture Sensor - to detect water level in soil
  - LED light strips - to supply plant with needed light
  - Water pump - to supply plant with water
  - Voltage Regulator - water pump needs more voltage than what the Nano can supply
  - Relais - works like a light switch

#### Calibrate humidity sensor
1. Figure out the output value in completely dry environment (here: $363$)
2. Figure out the output value while submerged in water (here: $699$)

These values correspond to 0% humidity and 100% humidity:
   Create a linear function that maps these values:
   $f(x) = mx + b = -0.29761x + 208.0357$

#### Relay



Subsequently we began with the work with the relay, which we needed because the Arduino itself can't provide sufficient voltage. Here we firstly tried to understand it by using it not connected to anything and after connected to the water pump. Then we saw that the tube of the water pump was too long and cut a bit off. What we wanted to see was how much water would come out after a certain time and we decided that two seconds was the best time for watering the mint plant.

It was decided that we use GitHub so that we can work on the same file somewhat at the same time.

Then we realized the stabilising code we were using was not very good, so we decided to recode it. 

While that was happening we also looked at how the lights would work.
