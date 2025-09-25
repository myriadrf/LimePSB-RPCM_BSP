all:
	dtc -@ -I dts -O dtb -o mcp23017_ca23.dtbo mcp23017_ca23.dts

install:
	cp mcp23017_ca23.dtbo /boot/firmware/overlays/

clean:
	rm mcp23017_ca23.dtbo