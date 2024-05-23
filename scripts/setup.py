from utils.installer import Installer
from mac.setup_mac import MacInstaller
from linux.setup_linux import LinuxInstaller
import platform
import sys


def main() -> None:
    installer: Installer = None
    os: str = platform.system()

    if os == 'Darwin':
        print('info: host os "MacOS" detected')
        installer = MacInstaller(
            ['make', 'x86_64_elf_binutils', 'x86_64_elf_gcc', 'nasm', 'grub2'])
    elif os == 'Linux':
        print('info: host os "Linux" detected')
        installer = LinuxInstaller(
            ['make', 'x86_64_elf_binutils', 'x86_64_elf_gcc', 'nasm', 'grub2'])
    elif os == 'Windows':
        print('info: host os "Windows" detected')
        print('warn: please use wsl2 to build HydraOS')
        print('error: unsupported host os')
        sys.exit(1)
    else:
        print('error: unsupported host os')
        sys.exit(1)

    print('info: installing packages')
    if not installer.install():
        print('error: unsuccessfull setup process')
        sys.exit(1)


if __name__ == '__main__':
    main()
