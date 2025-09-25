all:
	dtc -@ -I dts -O dtb -o mcp3208.dtbo mcp3208.dts

install:
	cp mcp3208.dtbo /boot/firmware/overlays/

clean:
	rm mcp3208.dtbo