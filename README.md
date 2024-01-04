# Spectre Version Control System

## Introduction

Spectre is a specialized version control system designed to efficiently manage file versions, prioritizing performance, security, and user experience. It is complemented by SpectreGUI, which offers a seamless graphical interface for users who prefer interactive navigation.

## Technical Description

Spectre harnesses the power of modern C++ features, the filesystem library, and multi-threading to ensure rapid and dependable file handling. It also integrates zlib for dynamic file compression, effectively reducing storage overhead and enhancing overall system performance.

### Multithreading and Concurrency

In response to evolving demands, Spectre now incorporates multithreading capabilities for file compression. Mutexes and locks are strategically employed to manage concurrent access to repository data, guaranteeing thread safety and allowing multiple processes to operate simultaneously without the risk of data corruption or race conditions.

### File Compression

Spectre's file compression functionality utilizes the robust and high-speed zlib compression library. The `compressFile` and `decompressFile` functions seamlessly transform repository data, conserving valuable disk space and optimizing data transfers with improved efficiency.

### Unique Features

- **Ignore Command**: Distinguishing itself from Git, Spectre incorporates an ignore command directly into the CLI. This feature enables users to specify files for exclusion without the necessity of a separate `.spectreignore` file.
- **Save Command**: Introducing a unique and streamlined workflow, the save command combines `add` and `commit` operations into a single step, enhancing user efficiency.

## SpectreGUI

Developed using the Qt framework, SpectreGUI interacts with the Spectre system, providing an intuitive graphical interface. It maintains a consistent look and feel across diverse platforms while leveraging Qt's robust signal and slot mechanism to ensure responsive UI updates.

### Security Enhancement: Multithreading for File Compression

In response to evolving security needs, Spectre has implemented multithreading for file compression. This enhancement optimizes the compression process by parallelizing tasks, resulting in faster and more efficient file handling. The inclusion of multithreading adds an extra layer of performance improvement, aligning with Spectre's commitment to security and user satisfaction.
