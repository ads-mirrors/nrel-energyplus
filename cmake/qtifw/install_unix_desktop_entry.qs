function Component()
{
  Component.prototype.createOperations = function()
  {
    // Call default implementation
    component.createOperations();

    // Ensure this only runs on Linux
    var kernel = systemInfo.kernelType;
    if (kernel === "linux") {
      const installPath = installer.value("TargetDir"); // User-selected install path
      const homeDir = installer.value("HomeDir");
      const desktopFilePath = homeDir + "/.local/share/applications/energyplus.25.1.desktop";
      const templateFile = installPath + "/share/applications/energyplus.desktop.in";

      // Delay execution by using addOperation() after install is complete
      component.addOperation("Execute", "/bin/sh", ["-c",
        "if [ -f '" + templateFile + "' ]; then " +
        "sed 's|ENERGYPLUS_INSTALL_DIR|" + installPath + "|g; s|ENERGYPLUS_VERSION|25.1|g' '" + templateFile + "' > '" + desktopFilePath + "' && " +
        "chmod 644 '" + desktopFilePath + "'; " +
        "else echo 'Error: .desktop template file missing'; fi"
      ]);
    }
  }
}
