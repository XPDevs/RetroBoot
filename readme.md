# RetroBoot

**RetroBoot** is a lightweight compatibility layer designed to bridge the gap between modern UEFI firmware and legacy BIOS-based operating systems. Its goal is simple: allow older operating systems that expect a traditional BIOS environment to boot and run on modern UEFI hardware **without modifying the operating system itself**.

Instead of requiring patches, custom builds, or firmware changes, RetroBoot provides a small UEFI application that prepares and emulates the environment these legacy systems expect. This makes it possible to experiment with, preserve, and run classic operating systems on contemporary machines that no longer support BIOS booting.

---

## Overview

Modern computers use **UEFI firmware**, which replaced the traditional **BIOS** interface used by older operating systems. Many legacy systems depend on BIOS interrupts, memory layouts, and boot behaviour that UEFI does not provide natively. As a result, these operating systems cannot normally start on UEFI-only hardware.

RetroBoot solves this by acting as a **compatibility bridge** between the firmware and the operating system.

When launched, RetroBoot performs the following sequence:

1. **Boot Environment Detection**  
   The application determines whether the system is running in BIOS compatibility mode or native UEFI mode.

2. **BIOS Mode Handling**  
   If the system is already operating in BIOS mode, RetroBoot displays a message indicating that the compatibility layer is unnecessary and safely halts.

3. **UEFI Mode Initialisation**  
   When running under UEFI, RetroBoot begins preparing a minimal BIOS-like execution environment.

4. **Legacy Boot Emulation**  
   The program loads the legacy boot code and provides the expected runtime conditions required for BIOS-based operating systems.

5. **Operating System Handoff**  
   Control is passed to the legacy OS bootloader, allowing it to continue the boot process as if it were running on a traditional BIOS system.

---

## Design Goals

RetroBoot is designed around several core principles:

- **Compatibility First**  
  Legacy operating systems should run *without modification* whenever possible.

- **Minimalism**  
  The project focuses on being lightweight and easy to understand, avoiding unnecessary complexity.

- **Transparency**  
  The boot process should remain clear and predictable for developers, hobbyists, and system researchers.

- **Hardware Preservation**  
  Many older operating systems become unusable on modern hardware. RetroBoot aims to help keep them accessible.

---

## Key Features

- Boot legacy BIOS-based operating systems on UEFI hardware  
- No modification required for the target operating system  
- Lightweight standalone UEFI application  
- Automatic environment detection  
- Clear diagnostic messages during boot  
- Designed for experimentation and OS development

---

## Project Status

RetroBoot is currently in **early development** and focuses on building the core boot compatibility layer. Features and behaviour may change as the project evolves.

The primary goal at this stage is to establish a reliable method for transitioning from a UEFI execution environment to a BIOS-compatible runtime suitable for legacy boot code.

---

## Intended Use Cases

RetroBoot can be useful for:

- Running older operating systems on modern machines  
- Operating system development and testing  
- Studying legacy boot processes  
- Preserving historical software environments  
- Experimenting with firmware-level compatibility layers

---

## Architecture Overview

At a high level, the RetroBoot boot process follows this structure:


UEFI Firmware
↓
RetroBoot UEFI Application
↓
Environment Detection
↓
BIOS Compatibility Layer
↓
Legacy Bootloader
↓
Legacy Operating System


This architecture allows RetroBoot to act as a controlled transition point between the modern firmware world and the expectations of older operating systems.

---

## Getting Started

Documentation for building and running RetroBoot will be provided in the sections below.

Further details include:

- Building the project
- Running RetroBoot on UEFI systems
- Boot configuration examples
- Development notes
- Contributing guidelines

---

## Philosophy

RetroBoot is built with the idea that **older software should not become unusable simply because hardware evolves**. By providing a small compatibility bridge, modern systems can continue to run and experiment with operating systems that were originally designed for entirely different firmware environments.

---
