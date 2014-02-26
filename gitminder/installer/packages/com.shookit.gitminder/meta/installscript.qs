 function Component()
 {
     // constructor
 }

 Component.prototype.isDefault = function()
 {
     // select the component by default
     return true;
 }

 Component.prototype.createOperations = function()
 {
     try {
         // call the base create operations function
         component.createOperations();
     } catch (e) {
         print(e);
     }
	 if (installer.value("os") === "win") {
        component.addOperation("CreateShortcut", "@TargetDir@/gitminder.exe", "@StartMenuDir@/gitminder.lnk",
            "workingDirectory=@TargetDir@",
            "iconId=2");

		component.addOperation("CreateShortcut", "@TargetDir@/gitminder.exe", "@DesktopDir@/gitminder.lnk",
            "workingDirectory=@TargetDir@",
            "iconId=3");
    }
 }