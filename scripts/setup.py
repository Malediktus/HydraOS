import mac.setup_mac as mac
import platform
import sys


class Installer():
    def __init__(self, packages: list) -> None:
        self.packages = packages

    def install(self) -> bool:
        pass


def main() -> None:
    installer: Installer = None
    os: str = platform.system()

    if os == 'Darwin':
        print('info: host os "MacOS" detected')
        installer = mac.MacInstaller(
            ['make', 'x86_64_elf_binutils', 'x86_64_elf_gcc', 'nasm', 'grub2'])
    elif os == 'Linux':
        print('info: host os "Linux" detected')
    elif os == 'Windows':
        print('info: host os "Windows" detected')
    else:
        print('error: unsupported host os')
        sys.exit(1)

    print('info: installing packages')
    if not installer.install():
        print('error: unsuccessfull setup process')
        sys.exit(1)


if __name__ == '__main__':
    main()
