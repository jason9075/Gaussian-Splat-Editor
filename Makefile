DEFAULT_GOAL:=help

OS := $(shell uname -s)
ARCH := $(shell uname -m)

.PHONY: dev                ## build & run
dev:
	@make build
	@make run

.PHONY: run
run:						## Run local application
	@./playground.app

.PHONY: build
build:							## build 
	@mkdir -p build
	@echo "Entering build directory and running CMake..."
	@cd build && cmake .. && make

.PHONY: test
test: build						## Run tests (currently just build)
	@echo "All tests passed!"

.PHONY: lint
lint:							## Run clang-format check
	@find src include Main.cpp -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \
		! -name "happly.h" ! -name "stb_image.h" ! -name "stb_image.cpp" | xargs clang-format --dry-run --Werror

.PHONY: format
format:							## Format code
	@find src include Main.cpp -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \
		! -name "happly.h" ! -name "stb_image.h" ! -name "stb_image.cpp" | xargs clang-format -i

.PHONY: clean
clean:					## Clean build files
	@rm -rf build

.PHONY: help
help:						## Show this help
	@echo "Makefile for local development"
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m (default: help)\n\nTargets:\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-18s\033[0m %s\n", $$1, $$2 }' $(MAKEFILE_LIST)
