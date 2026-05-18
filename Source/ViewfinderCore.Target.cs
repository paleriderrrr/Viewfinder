using UnrealBuildTool;
using System.Collections.Generic;

public class ViewfinderCoreTarget : TargetRules
{
	public ViewfinderCoreTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ViewfinderCore");
	}
}
