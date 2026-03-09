# LogiKnotting
LogiKnotting is an open-source topological knot design tool.
Born in France, it aims to provide a rigorous and logical approach to cylindrical and structured knot construction.
---
## Features
- Append-only truth model
- Deterministic segment and crossing logic
- Cylindrical ribbon model
- Millimetric snap grid
- Undo / Redo
- Persistent design time tracking
- Languages supported
- fr en de nl es it nl_BE pt pt_BR ja zh_CN zh_TW el ru uk fi da sv no pl cs sk hu ro bg tr ar he ko vi
---
## Philosophy
LogiKnotting distinguishes between:
- **Truth** (points, segments, crossings)
- **Context** (visual aids, helpers, references)
The core model is deterministic, append-only, and reproducible.
---
## Build
### Requirements
- Qt 6
- CMake 3.16 or newer
- C++17 compatible compiler
### Build Commands

    cmake -S . -B build
    cmake --build build
The executable will be generated in the `build` directory.
---
## License
LogiKnotting is released under the GNU General Public License v3.
See the `LICENSE` file for full terms.
---
## Author
Norbert TRUPIANO
