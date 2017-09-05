.PHONY: default

default: build

clean:
	pebble clean

build: clean
	pebble build

install: build
	pebble install -v --phone ${PHONE}
