# Contributing to C-LHF&D-M

Thank you for your interest in contributing to C-LHF&D-M!

## Development Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/Saladin5101/C-LHF-D-M.git
   cd C-LHF-D-M
   ```

2. Build the project:
   ```bash
   make clean
   make all
   ```

3. Run tests:
   ```bash
   make test
   ```

## Code Guidelines

- Use `gcc` compatible C code
- Follow K&R style with 4-space indentation
- Write clear comments for complex logic
- Include docstrings for public functions
- Keep functions focused and modular

## Testing

- Write unit tests for new features
- Place tests in `tests/` directory
- Ensure all tests pass before submitting PR

## Commit Messages

- Use clear, descriptive commit messages
- Reference issues when applicable
- Follow the format: `[type]: brief description`
  - Types: feature, bugfix, docs, refactor, test

## License

By contributing, you agree that your contributions will be licensed under the GNU AGPL v3 license.
