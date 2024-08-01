# Drop counter CAVEDROP
Drop counter for speleothems in caves
CAVEDROP
Andrés Ros
CaveDrop is an autonomous system designed to monitor the drip rate in caves, specifically those contributing to the formation of speleothems. This system uses piezoelectric sensors to detect and record drop impacts, allowing researchers to collect valuable data on the frequency and distribution of these drops over various periods. CaveDrop is a solution designed for speleological studies that quantifies the number of drops impacting a surface over time. These data are crucial for understanding the processes of speleothem formation and for evaluating environmental conditions within caves.
The hardware configuration uses only the necessary and minimal elements to optimize power consumption and final cost. It includes a low-power ATmega328PB microcontroller based on the enhanced RISC architecture of AVR®, a piezoelectric sensor, several switches to adjust the data logging interval (1 minute, 10 minutes, or 1 hour), and a temporary data storage system in a buffer before cyclically transferring it to a removable microSD card. The device's software counts the detected drops, avoids false positives with a 0.2-second delay, and stores the data in TXT format for further analysis. All the hardware is housed inside a waterproof container, designed to operate in the often humid and adverse conditions of caves.
