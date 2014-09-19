MUCCC flipdots controller for ATtiny2313
========================================

Schematics
----------

	   .------.___.-----.
	--[|/RESET 1  20 VCC|]--- 5V         .--.________________________________
	--[|RXD    2  19    |                |xx|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/
	--[|TXD    3  18 PB6|]----- ROW_DATA |xx| GND -.   (red side)           \
	   |       4  17 PB5|]------- STROBE |xx| GND -+                        /
	   |       5  16 PB4|]----- BLACK_OE |xx| GND -+                        `
	   |       6  15 PB3|]----- WHITE_OE |xx| GND -+   (cable comes from here)
	   |       7  14 PB2|]---- COL_CLOCK |xx| GND -+                        ,
	   |       8  13 PB1|]---- ROW_CLOCK |xx| GND -+------.                 \
	   |       9  12 PB0|]----- COL_DATA |xx| GND -+    __|__               /
	.-[|GND   10  11    |                |xx| GND -+     ---                \
	|  '----------------'                |xx|_____ | ____ ~ ________________/
	|                                    '--'      |
	|______________________________________________|
