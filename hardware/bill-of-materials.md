# Bill of Materials (BOM)

Complete parts list for the ESP32 Vehicle Monitor System.

## ESP32C6 Sender (Engine Bay Unit)

| Component | Description | Quantity | Approx. Price | Supplier Links |
|-----------|-------------|----------|---------------|----------------|
| **Seeed XIAO ESP32C6** | Main microcontroller | 1 | $7 | [Seeed](https://www.seeedstudio.com/Seeed-XIAO-ESP32C6-p-5884.html), [Amazon](https://amazon.com) |
| **Adafruit MAX31856** | Precision thermocouple amplifier (Part #3263) | 1 | $18 | [Adafruit](https://www.adafruit.com/product/3263), [DigiKey](https://www.digikey.com) |
| **K-Type Thermocouple** | Temperature sensor probe (appropriate length) | 1 | $10-30 | [Adafruit](https://www.adafruit.com/product/3245), [Amazon](https://amazon.com) |
| **ADS1115** | 16-bit ADC module | 1 | $8 | [Adafruit](https://www.adafruit.com/product/1085), [Amazon](https://amazon.com) |
| **0-100 PSI Pressure Sensor** | Oil pressure transducer (0.5-4.5V output) | 1 | $15-25 | [Amazon](https://amazon.com), [eBay](https://ebay.com) |
| **SSD1306 OLED Display** | 128x64 I2C display | 1 | $8 | [Amazon](https://amazon.com), [AliExpress](https://aliexpress.com) |
| **2.2kΩ Resistor** | Voltage divider (1/4W or 1/8W) | 1 | <$1 | Local electronics store |
| **4.7kΩ Resistor** | Voltage divider (1/4W or 1/8W) | 1 | <$1 | Local electronics store |
| **Breadboard** | For prototyping (or custom PCB) | 1 | $5 | [Amazon](https://amazon.com) |
| **Jumper Wires** | Male-male, male-female assortment | 1 set | $5 | [Amazon](https://amazon.com) |
| **USB-C Cable** | For programming XIAO ESP32C6 | 1 | $5 | [Amazon](https://amazon.com) |
| **Enclosure** | Weatherproof box for engine bay (optional) | 1 | $10-20 | [Amazon](https://amazon.com) |

**Sender Subtotal:** ~$90-130

### Notes on Sender Components:

**MAX31856:**
- Use the Adafruit version for reliable libraries and support
- Requires K-type thermocouple (not included)
- Operating temperature: -200°C to +1350°C

**K-Type Thermocouple:**
- Choose length appropriate for your installation
- Consider high-temperature wire insulation for engine bay
- Thread size should match your oil pan/adapter fitting
- Typical: 1/8" NPT thread, 6" to 12" probe length

**Pressure Sensor:**
- Must output 0.5-4.5V for 0-100 PSI range
- Common types: 1/8" NPT thread
- Check your engine's normal oil pressure range
- Some sensors include electrical connector

**Enclosure:**
- Must be heat-resistant for engine bay mounting
- Consider IP65 or better waterproof rating
- Needs cable glands for thermocouple and sensor wires

---

## CYD Display (Dashboard Unit)

| Component | Description | Quantity | Approx. Price | Supplier Links |
|-----------|-------------|----------|---------------|----------------|
| **Cheap Yellow Display (CYD)** | ESP32 + 320x240 ILI9341 TFT touchscreen | 1 | $15-20 | [AliExpress](https://aliexpress.com), [Amazon](https://amazon.com) |
| **USB Cable** | Micro-USB or USB-C (depending on CYD version) | 1 | $5 | [Amazon](https://amazon.com) |
| **Mounting Hardware** | Dashboard mount or adhesive (optional) | 1 | $5-15 | [Amazon](https://amazon.com) |

**Display Subtotal:** ~$20-40

### Notes on Display:

**CYD (Cheap Yellow Display):**
- Search for "ESP32 2432S028R" or "Cheap Yellow Display"
- Comes complete with ESP32, TFT, touch controller, SD card slot
- Usually includes USB programming capability
- Some versions have different USB ports (micro-USB vs USB-C)
- All-in-one solution, no additional components needed

---

## Laptop GPS System

| Component | Description | Quantity | Approx. Price | Supplier Links |
|-----------|-------------|----------|---------------|----------------|
| **U-blox GPS USB Dongle** | USB GPS receiver (U-blox chipset recommended) | 1 | $20-40 | [Amazon](https://amazon.com), [eBay](https://ebay.com) |
| **Laptop** | Debian 13 Linux or compatible | 1 | (existing) | - |
| **USB Cable** | USB-A to Micro-USB/USB-C for CYD connection | 1 | $5 | [Amazon](https://amazon.com) |

**GPS System Subtotal:** ~$25-45

### Notes on GPS:

**GPS Dongle:**
- U-blox chipsets recommended for gpsd compatibility
- Look for: VK-162, BU-353S4, or similar
- USB interface simplifies installation
- External antenna models available for better reception
- Must be compatible with gpsd on Linux

**Laptop:**
- Any laptop running Debian 13 or compatible Linux distribution
- Raspberry Pi can also be used as alternative
- Must support gpsd daemon
- USB ports required (GPS + CYD connection)

---

## Optional/Recommended Accessories

| Component | Description | Quantity | Approx. Price | Purpose |
|-----------|-------------|----------|---------------|---------|
| **NPT Fittings** | Oil pan adapters, T-fittings | As needed | $10-30 | Mount sensors in oil system |
| **Teflon Tape** | Thread sealant | 1 roll | $2 | Seal NPT threads |
| **Heat Shrink Tubing** | Various sizes | 1 set | $10 | Protect wire connections |
| **Electrical Tape** | High-temp rated | 1 roll | $5 | Secure wiring in engine bay |
| **Zip Ties** | Cable management | 1 pack | $5 | Organize wiring |
| **Crimps/Connectors** | Weatherproof electrical connectors | As needed | $10-20 | Removable connections |
| **Multimeter** | For testing/troubleshooting | 1 | $20-50 | Verify voltages and connections |
| **Spare Fuses** | Inline fuses for power | 3-5 | $5 | Protect against shorts |
| **12V Power Supply** | Cigarette lighter adapter or hardwire | 1 | $10-15 | Power the sender unit |
| **5V Power Supply** | USB power adapter | 1 | $5-10 | Power the CYD display |

**Accessories Subtotal:** ~$80-175

---

## Total System Cost

| Section | Cost Range |
|---------|------------|
| ESP32C6 Sender | $90-130 |
| CYD Display | $20-40 |
| Laptop GPS | $25-45 |
| Optional Accessories | $80-175 |
| **Total** | **$215-390** |

*Prices are approximate and vary by supplier and region. Many components available cheaper from AliExpress with longer shipping times.*

---

## Tools Required

Not included in BOM, but needed for assembly:

- Soldering iron and solder (for header pins if not pre-soldered)
- Wire strippers
- Screwdrivers (for terminal blocks)
- Multimeter (for testing)
- Drill and bits (for mounting enclosure)
- Wrenches (for NPT fittings in oil system)

---

## Recommendations

### Budget Build (~$215):
- Order from AliExpress for cheaper components
- Use existing laptop
- Basic enclosure or no enclosure (careful mounting)
- Minimal accessories

### Quality Build (~$300):
- Order key components from Adafruit/DigiKey for reliability
- Good weatherproof enclosure
- Quality GPS dongle with external antenna
- Proper connectors and strain relief

### Premium Build (~$390):
- All premium components
- Custom PCB instead of breadboard (add $50)
- Professional pressure transducer with connector
- High-quality weatherproof enclosures
- Complete accessory kit

---

## Supplier Notes

**Adafruit:**
- Reliable, well-documented components
- Good customer support
- Higher prices but includes tutorials
- Ships from USA

**Amazon:**
- Fast shipping (Prime)
- Mix of quality levels
- Read reviews carefully
- Easy returns

**AliExpress:**
- Lowest prices
- Long shipping times (2-6 weeks)
- Good for non-critical components
- Check seller ratings

**DigiKey/Mouser:**
- Professional component suppliers
- Excellent selection
- Technical datasheets
- Higher prices, fast shipping

---

## Maintenance Parts

Consider keeping spares of these failure-prone items:

- Extra jumper wires
- Spare thermocouple
- Backup pressure sensor
- Extra USB cables
- Replacement OLED display
- Spare ESP32C6 module

---

**Last Updated:** January 2026  
**Currency:** USD  
**Note:** Prices are estimates and subject to change. Check current prices before ordering.
