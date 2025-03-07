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
      const desktopFilePath = installer.value("HomeDir") + "/.local/share/applications/energyplus.desktop";
      const templateFile = installPath + "/share/applications/energyplus.desktop.in";

      // Check if the template file exists before reading
      if (!installer.fileExists(templateFile)) {
        console.log("Error: .desktop template file not found at " + templateFile);
        return;
      }

      // Read the template file
      let content = installer.readFile(templateFile);
      if (!content) {
        console.log("Error: Unable to read .desktop template");
        return;
      }

      // Replace placeholders
      content = content.replace(/@INSTALL_DIR@/g, installPath);
      content = content.replace(/@VERSION@/g, "25.1");

      // Use a temporary file
      const tempFile = installPath + "/energyplus.desktop.tmp";
      const writeCommand = "echo '" + content.replace(/'/g, "'\\''") + "' > " + tempFile;

      // Execute the command to write the file
      var writeSuccess = installer.execute("/bin/sh", ["-c", writeCommand]);
      if (!writeSuccess) {
        console.log("Error: Unable to write .desktop file");
        return;
      }

      // Move the temporary file to the final destination
      installer.execute("mv", [tempFile, desktopFilePath]);

      // Set correct permissions
      installer.execute("chmod", ["644", desktopFilePath]);

      console.log("Desktop entry created at: " + desktopFilePath);
    }
  }
}
