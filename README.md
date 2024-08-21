
# Cache Simulator

This project implements a cache simulator in C, designed to demonstrate caching mechanisms with Least Frequently Used (LFU) cache replacement policy. The simulator handles basic operations like reading and writing bytes, and is capable of simulating the behavior of caches of different sizes and configurations.

## Features

- Simulate a fully associative cache.
- LFU (Least Frequently Used) replacement policy.
- Operations to read and write bytes to the cache.
- Configurable cache parameters (number of sets, cache lines per set, block size, and tag size).
- Debug functions to print the state of the cache.

## Installation

To get started with this cache simulator, clone the repository to your local machine using:

```bash
git clone https://github.com/yourusername/cache-simulator.git
```

Navigate to the directory where the project was cloned:

```bash
cd cache-simulator
```

## Compilation

Compile the simulator using gcc or any standard C compiler:

```bash
gcc -o cache_simulator main.c
```

## Usage

Run the simulator:

```bash
./cache_simulator
```

Follow the on-screen prompts to input the cache configuration parameters and the data size. The simulator will then allow you to perform read operations and will display the state of the cache.

## Configuration

You will be prompted to enter:
- `s`: Number of set index bits.
- `t`: Number of tag bits.
- `b`: Number of block offset bits.
- `E`: Number of lines per set (associativity).

These parameters allow you to simulate different cache configurations and observe how changes affect cache behavior.

## Contributing

Contributions are welcome! Please fork the repository, make your changes, and submit a pull request with a clear description of what your changes do.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
