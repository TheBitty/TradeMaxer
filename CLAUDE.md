# CLAUDE.md

This file contains instructions and context for Claude Code to help with this project.

## Project Overview
Trading System - A financial trading application built with C++ and Python

## Development Commands
- Build: `make` or `cmake --build build/`
- Test: `make test` or `pytest`
- Clean: `make clean`

## Project Structure
- `/src` - Main source code
- `/tests` - Test files
- `/docs` - Documentation

## Coding Standards
- Follow existing C++/Python conventions
- Use consistent naming patterns
- Maintain proper error handling
- Write tests for new features

## Architecture Principles
- **Main function as entry point only**: The main function should serve solely as an entry point, delegating all logic to other modules
- **Modularity is key**: Each component should have its own file/module for better organization and maintainability
- **Multi-process design**: Design with multiple processes in mind - components should be loosely coupled and communicate through well-defined interfaces
- **Separation of concerns**: Each module should have a single responsibility
- **Inter-process communication**: Use appropriate IPC mechanisms (pipes, sockets, shared memory) for process communication

## Notes
- This is a trading system project
- Focus on financial data accuracy and security
- Ensure proper validation of trading operations
- Design for concurrent processing and scalability