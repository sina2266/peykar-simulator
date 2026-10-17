import platform
import struct
import os
import platform

def get_library_archs(lib_path: str) -> tuple[str, list[str]]:
    """Determine architecture based on file type and header."""
    _, ext = os.path.splitext(lib_path)

    # ELF (Linux .so files)
    if ext == ".so" or ".so." in str(lib_path):
        with open(lib_path, 'rb') as f:
            f.seek(5)
            endian = '<' if f.read(1)[0] == 1 else '>'
            f.seek(18)
            machine = struct.unpack(endian + 'H', f.read(2))[0]

        arch_map = {
            0x03: 'i386',
            0x3E: 'x86_64',
            0xB7: 'aarch64',
            0x28: 'armv7',
        }
        return "Linux", [normalize_arch(arch_map.get(machine, f'unknown (0x{machine:x})'))]

    # Mach-O (macOS .dylib files)
    elif ext == '.dylib':
        with open(lib_path, 'rb') as f:
            # Check for fat binary magic
            magic = f.read(4)
            if magic == b'\xca\xfe\xba\xbe' or magic == b'\xbf\xba\xfe\xca':
                # Fat binary - list all architectures
                f.seek(4)
                num_archs = struct.unpack('>I', f.read(4))[0]
                archs = set()
                for i in range(num_archs):
                    cpu_type = struct.unpack('>I', f.read(4))[0]
                    f.read(4)  # Skip cpu_subtype
                    f.read(4)  # Skip offset
                    f.read(4)  # Skip size
                    f.read(4)  # Skip align

                    arch_map = {
                        0x00000007: 'i386',
                        0x01000007: 'x86_64',
                        0x0100000C: 'aarch64',
                    }
                    archs.add(normalize_arch(arch_map.get(cpu_type, f'unknown (0x{cpu_type:x})')))
                return "Darwin", list(archs)
            else:
                # Single-arch Mach-O - determine endianness
                f.seek(4)
                cpu_type_bytes = f.read(4)
                # Try little-endian first (most common)
                cpu_type = struct.unpack('<I', cpu_type_bytes)[0]

                arch_map = {
                    0x00000007: 'i386',
                    0x01000007: 'x86_64',
                    0x0100000C: 'aarch64',
                }
                return "Darwin", [ normalize_arch(arch_map.get(cpu_type, f'unknown (0x{cpu_type:x})')) ]

    # PE (Windows .dll files)
    elif ext == '.dll':
        with open(lib_path, 'rb') as f:
            f.seek(0x3c)
            pe_offset = struct.unpack('<I', f.read(4))[0]
            f.seek(pe_offset + 4)
            machine = struct.unpack('<H', f.read(2))[0]

        arch_map = {
            0x014C: 'i386',
            0x8664: 'x86_64',
            0xAA64: 'aarch64',
        }
        return "Windows", [ normalize_arch(arch_map.get(machine, f'unknown (0x{machine:x})')) ]

    return 'unknown', [ ]

def get_all_valid_libraries_from_dir(lib_dir: str):
    dir_files = [str(full_file) for f in os.listdir(lib_dir) if os.path.isfile(full_file := os.path.join(lib_dir, f))]
    return filter_all_valid_libraries(dir_files)


def filter_all_valid_libraries(lib_path_list: list[str]) -> list[str]:
    if not lib_path_list:
        raise ValueError("A list must be provided, `None` is not allowed.")

    system_name = platform.system()
    system_arch = normalize_arch(platform.machine())
    valid_libraries = []
    for lib_path in lib_path_list:
        required_system_name, shared_library_archs = get_library_archs(lib_path)
        if system_arch in shared_library_archs and required_system_name in system_name:
            valid_libraries.append(lib_path)
    return valid_libraries

def normalize_arch(machine_str=None):
    if machine_str is None:
        machine_str = platform.machine().lower()

    # Normalize common variants
    if machine_str in ('x86_64', 'amd64', 'x64'):
        return 'x86_64'
    elif machine_str in ('aarch64', 'arm64'):
        return 'aarch64'
    elif machine_str in ('i386', 'i486', 'i586', 'i686', 'x86'):
        return 'i386'
    elif machine_str.startswith('arm'):
        return 'arm'
    elif machine_str.lower() != machine_str:
        return normalize_arch(machine_str.lower())
    raise OSError(f"Unknown machine type detected: `{machine_str}`")