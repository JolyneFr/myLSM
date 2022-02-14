target =

.PHONY: config
config:
	cmake -B build

.PHONY: clean
clean:
	rm -rf build bin

.PHONY: build-tests
build-tests:
	cmake --build build

.PHONY: build-all
build-all:
	cmake --build build

.PHONY: build
build:
ifeq ($(target),)
	echo "build command must specify target"
else
	cmake --build build --target $(target)
endif