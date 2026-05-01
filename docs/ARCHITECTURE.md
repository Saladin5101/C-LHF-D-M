# C-LHF&D-M Architecture

## Overview
C-LHF&D-M (C Language Header Files and Dependency Manager) is a package manager for C language development that enables cross-platform dependency management.

## Project Structure

- **src/**: Core source code
  - `main.c` - Command-line interface entry point
  - `package.c` - Package management operations
  
- **include/**: Header files
  - `clhfm.h` - Main API header
  
- **tests/**: Unit and integration tests
  - `test_main.c` - Test runner
  
- **build/**: Build artifacts (generated)
  
- **bin/**: Binary executables (generated)

- **docs/**: Documentation
  - `ARCHITECTURE.md` - This file
  - Design specifications and implementation guides

- **scripts/**: Utility scripts
  - Installation and setup scripts

- **config/**: Configuration files
  - Default settings and templates

- **examples/**: Example projects using C-LHF&D-M

## Core Components

### 1. CLI Interface (main.c)
- Command parsing and routing
- User interaction handling
- Help and version information

### 2. Package Management (package.c)
- Install: Fetch and install dependencies
- Remove: Uninstall packages
- Update: Check and update to latest versions
- Push: Publish custom packages
- Tag: Version management

### 3. Registry Client
- Connect to package registry
- Search for packages
- Download packages

### 4. Dependency Resolution
- Resolve package dependencies
- Check version compatibility
- Handle dependency conflicts

## Build System

The project uses `make` for building. Key targets:
- `make all` - Build the main executable
- `make install` - Install to system
- `make test` - Run tests
- `make clean` - Clean build artifacts

## Development Workflow

1. Create feature branches
2. Implement features with corresponding tests
3. Run `make test` to verify
4. Update documentation
5. Submit pull request
