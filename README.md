# Project Name

This repository contains the embedded software for the ECE senior design project at UT Austin. The software is built on top of Nordic Semiconductor's NRF Connect SDK.

## Installation / Setup

Before you clone the repository, it is important that you setup the necessary toolchains for an NRF based project.
<<<<<<< HEAD
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
=======
To install the **necessary VSCODE extensions**, follow NRF's getting started guide [here](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/installation.html).
Ensure that you setup the **2.5.0** version of the toolchain.

After you have done this, clone this repository, and navigate to the `NRF connect` extension in your sidebar.
This extension will have all of the tools you need to develop, flash, and debug the board.

To setup, first, register our application. In the bottom of the left sidebar, there is a dialogue on first launch giving you the option to either `open` or `create` an application.
Hit `open` and then select the **application** folder.

After you this, that dialogue will be replaced by one asking you to create a build configuration. Follow these steps closely and carefully.
First, hit **create build configuration** For the board **ENSURE** that you select the **nrf52dk_nrf52832**.
Then, click on **add overlay** and select the file within the application folder (`application/nrf52dk_nrf52832.overlay`).
If pin assignments change, you will have to edit that file to reflect it, but hopefully that will have been done by whoever adjusted the pinout.

You are now setup to build, flash, and debug the board. Refer to the NRF vscode extension guides if you want to familiarize yourself with the environment.
>>>>>>> b697c3ecad344c042fc8f9efdd8df2b04b7229c3

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
