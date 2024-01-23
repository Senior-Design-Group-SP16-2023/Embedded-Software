# Project Name

This repository contains the embedded software for the ECE senior design project at UT Austin. The software is built on top of Nordic Semiconductor's NRF Connect SDK.

## Installation / Setup

Before you clone the repository, it is important that you setup the necessary toolchains for an NRF based project.
This project follows NRF's recommended workflow of building the project in an isolated environment managed by `west`. 
To install `west`, follow the instructions [here](https://docs.zephyrproject.org/latest/develop/west/install.html).
To install the **necessary VSCODE extensions**, follow NRF's getting started guide [here](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/installation.html).

After you have done both of those steps, verify that west is properly installed by running `west --version` in your terminal.
Once you have verified your west installation, run the following command in the directory where you want to create the project top-level directory:
```bash
west init -m git@github.com:Senior-Design-Group-SP16-2023/Embedded-Software.git Embedded-Software
```
This command will clone this repository and set it up as a **west workspace**.
Initially, within this workspace, you will see two folders: `.west` and `application`. The latter is where our code lives.

After you have done this, navigate into the newly created `Embedded-Software` directory, and run `west update`.
This will clone all necessary submodules and sdk dependencies into a folder called `external`. Do not modify this folder.
Currently, our `west.yml` file (within the `application` folder) specifies that we are using NRF Connect SDK version 2.5.1. If this 
needs to change, adjust the YAML file, and then re-run `west update`.

After you have done this, open the `application` folder in VSCODE. You are now ready to start developing.

## Development Workflow

The following is our standard development workflow for improving the software.

1. Fork the repository.
2. Create a new branch: `git checkout -b feature/your-feature`
3. Make your changes and commit them: `git commit -m 'Add your feature'`
4. Push to the branch: `git push origin feature/your-feature`
5. Submit a pull request.

## Software Architecture

WIP

## Building / testing the project

WIP