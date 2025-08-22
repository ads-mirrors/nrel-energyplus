from enum import IntEnum, StrEnum


class OS(StrEnum):
    """Operating Systems."""

    Windows = "Windows"
    Linux = "Linux"
    Mac = "Mac"


class MSVC(IntEnum):
    """Microsoft Visual Studio versions."""

    V16 = 16
    V17 = 17
    # V18 = 18  # Future version placeholder

    def generator_name(self) -> str:
        """Get the CMake generator name for the MSVC version."""
        return MSVC_GENERATOR_MAPPING[self]


# Alias mapping for common version names to MSVC enum, for argparse
MSVC_ALIAS_MAPPING = {
    "2017": MSVC.V16,
    "2022": MSVC.V17,
}

# Mapping from MSVC enum to CMake generator names
MSVC_GENERATOR_MAPPING = {
    MSVC.V16: "Visual Studio 16 2019",
    MSVC.V17: "Visual Studio 17 2022",
}


class Bitness(StrEnum):
    X86 = "x86"
    X64 = "x64"
    ARM64 = "arm64"
