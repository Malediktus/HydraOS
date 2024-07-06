import utils.cli as cli
from utils.installer import Installer
import subprocess
import os


class HomebrewManager:
    @staticmethod
    def _run_command_text(command):
        try:
            result = subprocess.run(
                command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            return result.stdout.strip()
        except subprocess.CalledProcessError as e:
            return e.stderr.strip()

    @staticmethod
    def _run_command(command):
        try:
            subprocess.run(command, check=True, text=True)
            return True
        except subprocess.CalledProcessError:
            return False

    @staticmethod
    def is_homebrew_installed():
        result = HomebrewManager._run_command_text(['which', 'brew'])
        return result != ''

    @staticmethod
    def install_homebrew():
        if HomebrewManager.is_homebrew_installed():
            return False

        os.system(
            '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"')

    @staticmethod
    def install_package(package_name):
        if not HomebrewManager.is_homebrew_installed():
            return False
        return HomebrewManager._run_command(['brew', 'install', package_name])

    @staticmethod
    def update_homebrew():
        if not HomebrewManager.is_homebrew_installed():
            return False
        return HomebrewManager._run_command(['brew', 'update'])

    @staticmethod
    def upgrade_package(package_name):
        if not HomebrewManager.is_homebrew_installed():
            return False
        return HomebrewManager._run_command(['brew', 'upgrade', package_name])

    @staticmethod
    def is_package_installed(package_name, version=None):
        if not HomebrewManager.is_homebrew_installed():
            return False
        installed_packages = HomebrewManager._run_command_text(
            ['brew', 'list', '--versions'])
        for line in installed_packages.split('\n'):
            parts = line.split()
            if parts[0] == package_name:
                if version is None:
                    return True
                elif version in parts[1:]:
                    return True
        return False


class MacInstaller(Installer):
    def __init__(self, packages: list) -> None:
        super().__init__(packages)

    def install(self) -> bool:
        if not HomebrewManager.is_homebrew_installed():
            if not cli.query_yes_no('homebrew is not installed... install it?'):
                return False

            HomebrewManager.install_homebrew()
        else:
            print('info: updating homebrew')
            if not HomebrewManager.update_homebrew():
                print('error: failed to update homebrew')
                return False

        for package in self.packages:
            try:
                if not getattr(self, f'install_{package}')():
                    return False
            except AttributeError:
                print(f'error: package {package} can not be install on MacOS')
                return False

        return True

    def install_make(self) -> bool:
        if HomebrewManager.is_package_installed('make'):
            print('info: upgrading "make"')
            if not HomebrewManager.upgrade_package('make'):
                print('error: failed to upgrade package "make"')
                return False

            return True
        if not cli.query_yes_no("make is not installed... install it?"):
            return False

        if not HomebrewManager.install_package('make'):
            print('error: failed to install package "make"')
            return False

        return True

    def install_gcc(self) -> bool:
        if HomebrewManager.is_package_installed('gcc'):
            print('info: upgrading "gcc"')
            if not HomebrewManager.upgrade_package('gcc'):
                print('error: failed to upgrade package "gcc"')
                return False

            return True
        if not cli.query_yes_no("gcc is not installed... install it?"):
            return False

        if not HomebrewManager.install_package('gcc'):
            print('error: failed to install package "gcc"')
            return False

        return True

    def install_gpp(self) -> bool:
        if HomebrewManager.is_package_installed('g++'):
            print('info: upgrading "g++"')
            if not HomebrewManager.upgrade_package('g++'):
                print('error: failed to upgrade package "g++"')
                return False

            return True
        if not cli.query_yes_no("g++ is not installed... install it?"):
            return False

        if not HomebrewManager.install_package('g++'):
            print('error: failed to install package "g++"')
            return False

        return True

    def install_x86_64_elf_binutils(self) -> bool:
        if HomebrewManager.is_package_installed('x86_64-elf-binutils'):
            print('info: upgrading "x86_64-elf-binutils"')
            if not HomebrewManager.upgrade_package('x86_64-elf-binutils'):
                print('error: failed to upgrade package "x86_64-elf-binutils"')
                return False

            return True
        if not cli.query_yes_no("x86_64-elf-binutils is not installed... install it?"):
            return False

        if not HomebrewManager.install_package('x86_64-elf-binutils'):
            print('error: failed to install package "x86_64-elf-binutils"')
            return False

        return True

    def install_x86_64_elf_gcc(self) -> bool:
        if HomebrewManager.is_package_installed('x86_64-elf-gcc'):
            print('info: upgrading "x86_64-elf-gcc"')
            if not HomebrewManager.upgrade_package('x86_64-elf-gcc'):
                print('error: failed to upgrade package "x86_64-elf-gcc"')
                return False

            return True
        if not cli.query_yes_no("x86_64-elf-gcc is not installed... install it?"):
            return False

        if not HomebrewManager.install_package('x86_64-elf-gcc'):
            print('error: failed to install package "x86_64-elf-gcc"')
            return False

        return True

    def install_nasm(self) -> bool:
        if HomebrewManager.is_package_installed('nasm'):
            print('info: upgrading "nasm"')
            if not HomebrewManager.upgrade_package('nasm'):
                print('error: failed to upgrade package "nasm"')
                return False

            return True
        if not cli.query_yes_no("nasm is not installed... install it?"):
            return False

        if not HomebrewManager.install_package('nasm'):
            print('error: failed to install package "nasm"')
            return False

        return True
    
    def install_xorriso(self) -> bool:
        if HomebrewManager.is_package_installed('xorriso'):
            print('info: upgrading "xorriso"')
            if not HomebrewManager.upgrade_package('xorriso'):
                print('error: failed to upgrade package "xorriso"')
                return False

            return True
        if not cli.query_yes_no("xorriso is not installed... install it?"):
            return False

        if not HomebrewManager.install_package('xorriso'):
            print('error: failed to install package "xorriso"')
            return False

        return True

    def is_grub2_installed(self) -> bool:
        try:
            result = subprocess.run(
                ['grub-mkrescue', '--version'], capture_output=True, text=True)
            output = result.stdout.strip()
            return 'grub-mkrescue (GRUB) 2.' in output
        except FileNotFoundError:
            return False

    def install_grub2(self) -> bool:
        required_packages = ['pkg-config', 'm4', 'libtool',
                             'automake', 'autoconf', 'objconv', 'gawk']

        if self.is_grub2_installed():
            return True

        for package in required_packages:
            if HomebrewManager.is_package_installed(package):
                print(f'info: upgrading "{package}"')
                if not HomebrewManager.upgrade_package(package):
                    print(f'error: failed to upgrade package "{package}"')
                    return False
            else:
                if not cli.query_yes_no(f"{package} is not installed... install it?"):
                    return False

                if not HomebrewManager.install_package(package):
                    print(f'error: failed to install package "{package}"')
                    return False

        try:
            subprocess.run(['sh', 'mac/install_grub2.sh'], check=True)
            return True
        except subprocess.CalledProcessError:
            print('error: failed to execute installation script for grub2')
            return False
