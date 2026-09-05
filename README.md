# CollatzArt

CollatzArt is a simple visualization program for the numerical sequences calculated from the **Collatz conjecture**. It provides tools to explore, render, and save these sequences in visually appealing ways.

## Features

- Calculate Collatz sequences up to the number **10,000**.  
- Render sequences using different algorithms. Currently developed renderers:
  - **Feather**
  - **Tree**
- Save rendering options to a file for later use.
- Save a list of render jobs for batch processing.
- Replay render jobs from a file (**screensaver mode**).
- **Full-screen mode** for immersive visualization.
- Interactive CLI for adjusting rendering options.

## Rendering Algorithms

CollatzArt draws inspiration from the following sources:
- [The Collatz Tree](https://www.algoritmarte.com/the-collatz-tree/) by Algoritmarte
- [A Graph of the Collatz Conjecture](https://www.reddit.com/r/dataisbeautiful/comments/8miru1/a_graph_of_the_collatz_conjecture_how_the_first/) on Reddit

Special thanks to the authors for sharing their beautiful rendering methods.

## Build Instructions

CollatzArt is available as a **C++ CMake project** with build scripts for **macOS** and **Windows**.

### How to Build
1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd CollatzArt
   ```
2. Execute the appropriate build script:
   - **macOS**: `./build.sh`
   - **Windows**: `./build.ps1`

### Dependencies

CollatzArt requires the **Simple and Fast Multimedia Library (SFML)** for rendering and window management.

#### macOS
- Install SFML using [Homebrew](https://brew.sh/):
  ```bash
  brew install sfml
  ```

#### Windows
- Download SFML from the [official website](https://www.sfml-dev.org/download.php).
- Ensure the SFML binaries are in your system `PATH` or linked correctly in your project.

#### Linux (if applicable)
- Install SFML using your package manager, e.g., for Ubuntu:
  ```bash
  sudo apt-get install libsfml-dev
  ```