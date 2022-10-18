Application {
    name: "Kernel"

    files: [
        "*.cpp",
        "*.c",
        "*.h",
        "*.S",
    ]
    install: true
    installDir: "/System"

    Group {
        files: [
            "Limine/*"
        ]
        qbs.install: true
        qbs.installDir: "/"
    }

    cpp.cxxLanguageVersion: "c++20"

    cpp.cxxFlags: [
        "-ffreestanding",
        "-fno-stack-protector",
        "-fno-stack-check",
        "-fno-lto",
        "-fno-pie",
        "-fno-pic",
        "-m64",
        "-march=x86-64",
        "-mabi=sysv",
        "-mno-80387",
        "-mno-mmx",
        "-mno-sse",
        "-mno-sse2",
        "-mno-red-zone",
        "-mcmodel=kernel",
        "-MMD",
        "-I.",
    ]
    cpp.linkerFlags: [
        "-nostdlib",
        "-static",
        "-melf_x86_64",
        "-zmax-page-size=0x1000",
        "-T"+sourceDirectory+"/Linker.ld",
    ]
    cpp.linkerMode: "manual"

    Depends { name: "cpp" }
}
