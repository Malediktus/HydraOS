import utils.cli as cli
from utils.installer import Installer
import subprocess
import os


class PackageManager:
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
    def check_linux_distribution():
        distro_info = PackageManager._run_command_text(['lsb_release', '-is'])
        return distro_info.strip() == 'Ubuntu'

    @staticmethod
    def is_apt_installed():
        return PackageManager.check_linux_distribution()

    @staticmethod
    def install_apt():
        if not PackageManager.is_apt_installed():
            print("error: this script is intended for Ubuntu systems only.")
            exit(1)

    @staticmethod
    def install_package(package_name):
        if not PackageManager.is_apt_installed():
            return False
        return PackageManager._run_command(['sudo', 'apt', 'install', '-y', package_name])

    @staticmethod
    def update_apt():
        if not PackageManager.is_apt_installed():
            return False
        return PackageManager._run_command(['sudo', 'apt', 'update'])

    @staticmethod
    def upgrade_package(package_name):
        if not PackageManager.is_apt_installed():
            return False
        return PackageManager._run_command(['sudo', 'apt', 'upgrade', '-y', package_name])

    @staticmethod
    def is_package_installed(package_name, version=None):
        if not PackageManager.is_apt_installed():
            return False
        installed_packages = PackageManager._run_command_text(
            ['dpkg', '-l', package_name])
        return package_name in installed_packages


class LinuxInstaller(Installer):
    def __init__(self, packages: list) -> None:
        super().__init__(packages)

    def install(self) -> bool:
        if not PackageManager.is_apt_installed():
            if not cli.query_yes_no('apt is not installed... install it?'):
                return False

            PackageManager.install_apt()
        else:
            print('info: updating apt')
            if not PackageManager.update_apt():
                print('error: failed to update apt')
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
        if PackageManager.is_package_installed('make'):
            print('info: upgrading "make"')
            if not PackageManager.upgrade_package('make'):
                print('error: failed to upgrade package "make"')
                return False

            return True
        if not cli.query_yes_no("make is not installed... install it?"):
            return False

        if not PackageManager.install_package('make'):
            print('error: failed to install package "make"')
            return False

        return True

    def install_gcc(self) -> bool:
        if PackageManager.is_package_installed('gcc'):
            print('info: upgrading "gcc"')
            if not PackageManager.upgrade_package('gcc'):
                print('error: failed to upgrade package "gcc"')
                return False

            return True
        if not cli.query_yes_no("gcc is not installed... install it?"):
            return False

        if not PackageManager.install_package('gcc'):
            print('error: failed to install package "gcc"')
            return False

        return True

    def install_gpp(self) -> bool:
        if PackageManager.is_package_installed('g++'):
            print('info: upgrading "g++"')
            if not PackageManager.upgrade_package('g++'):
                print('error: failed to upgrade package "g++"')
                return False

            return True
        if not cli.query_yes_no("g++ is not installed... install it?"):
            return False

        if not PackageManager.install_package('g++'):
            print('error: failed to install package "g++"')
            return False

        return True

    def is_x86_64_elf_binutils_installed(self) -> bool:
        try:
            result = subprocess.run(
                ['x86_64-elf-ld', '--version'], capture_output=True, text=True)
            output = result.stdout.strip()
            return 'GNU ld (GNU Binutils)' in output
        except FileNotFoundError:
            return False

    def install_x86_64_elf_binutils(self) -> bool:
        required_packages = ['make', 'bison', 'flex', 'libgmp3-dev', 'libmpc-dev', 'libmpfr-dev', 'texinfo']

        if self.is_x86_64_elf_binutils_installed():
            return True

        for package in required_packages:
            if PackageManager.is_package_installed(package):
                print(f'info: upgrading "{package}"')
                if not PackageManager.upgrade_package(package):
                    print(f'error: failed to upgrade package "{package}"')
                    return False
            else:
                if not cli.query_yes_no(f"{package} is not installed... install it?"):
                    return False

                if not PackageManager.install_package(package):
                    print(f'error: failed to install package "{package}"')
                    return False

        try:
            subprocess.run(['bash', 'linux/install_x86_64-elf-binutils.sh'], check=True)
            return True
        except subprocess.CalledProcessError:
            print('error: failed to execute installation script for x86_64-elf-binutils')
            return False

    def is_x86_64_elf_gcc_installed(self) -> bool:
        try:
            result = subprocess.run(
                ['x86_64-elf-gcc', '--version'], capture_output=True, text=True)
            output = result.stdout.strip()
            return 'x86_64-elf-gcc (GCC)' in output
        except FileNotFoundError:
            return False

    def install_x86_64_elf_gcc(self) -> bool:
        if self.is_x86_64_elf_gcc_installed():
            return True

        try:
            subprocess.run(['bash', 'linux/install_x86_64-elf-gcc.sh'], check=True)
            return True
        except subprocess.CalledProcessError:
            print('error: failed to execute installation script for x86_64-elf-gcc')
            return False

    def install_nasm(self) -> bool:
        if PackageManager.is_package_installed('nasm'):
            print('info: upgrading "nasm"')
            if not PackageManager.upgrade_package('nasm'):
                print('error: failed to upgrade package "nasm"')
                return False

            return True
        if not cli.query_yes_no("nasm is not installed... install it?"):
            return False

        if not PackageManager.install_package('nasm'):
            print('error: failed to install package "nasm"')
            return False

        return True
    
    def install_xorriso(self) -> bool:
        if PackageManager.is_package_installed('xorriso'):
            print('info: upgrading "xorriso"')
            if not PackageManager.upgrade_package('xorriso'):
                print('error: failed to upgrade package "xorriso"')
                return False

            return True
        if not cli.query_yes_no("xorriso is not installed... install it?"):
            return False

        if not PackageManager.install_package('xorriso'):
            print('error: failed to install package "xorriso"')
            return False

        return True

    def install_grub2(self) -> bool:
        if PackageManager.is_package_installed('grub2-common'):
            print('info: upgrading "grub2-common"')
            if not PackageManager.upgrade_package('grub2-common'):
                print('error: failed to upgrade package "grub2-common"')
                return False

            return True
        if not cli.query_yes_no("grub2-common is not installed... install it?"):
            return False

        if not PackageManager.install_package('grub2-common'):
            print('error: failed to install package "grub2-common"')
            return False
        
        if PackageManager.is_package_installed('grub-pc-bin'):
            print('info: upgrading "grub-pc-bin"')
            if not PackageManager.upgrade_package('grub-pc-bin'):
                print('error: failed to upgrade package "grub-pc-bin"')
                return False

            return True
        if not cli.query_yes_no("grub-pc-bin is not installed... install it?"):
            return False

        if not PackageManager.install_package('grub-pc-bin'):
            print('error: failed to install package "grub-pc-bin"')
            return False

        return True
