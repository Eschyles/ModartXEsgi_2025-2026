using UnrealBuildTool;
using System.Collections.Generic;

public class UprojectEditorTarget : TargetRules
{
	public UprojectEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Uproject");
	}
}
