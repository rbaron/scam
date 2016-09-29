

INPUT = scam.c
OUTPUT = scam

build: clean
	@echo "Compiling..."
	gcc $(INPUT) -o$(OUTPUT)

clean:
	@echo "Cleaning..."
	rm -f $(OUTPUT)
