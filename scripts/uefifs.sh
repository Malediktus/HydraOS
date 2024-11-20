#!/bin/bash

set -e

sudo mkdir -p /mnt/EFI/BOOT
sudo cp ../bootloader/build/bootloader.efi /mnt/EFI/BOOT

sudo cp startup.nsh /mnt/startup.nsh
