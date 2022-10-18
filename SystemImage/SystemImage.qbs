Product {
    type: ["iso"]

    property string piss: qbs.installRoot

    Rule {
        multiplex: true
        inputsFromDependencies: ["application"]

        prepare: {
            var command = new Command("xorriso", [
                "-as",
                "mkisofs",
                "-b",
                "/limine-cd.bin",
                "-no-emul-boot",
                "-boot-load-size",
                "4",
                "-boot-info-table",
                "--efi-boot",
                "/limine-cd-efi.bin",
                "-efi-boot-part",
                "--efi-boot-image",
                "--protective-msdos-label",
                product.piss,
                "-o",
                product.buildDirectory+"/Xesual.iso",
            ])
            return command
        }

        Artifact {
            filePath: "Xesual.iso"
            fileTags: ["iso"]
        }
    }
    Depends { name: "Kernel" }
}