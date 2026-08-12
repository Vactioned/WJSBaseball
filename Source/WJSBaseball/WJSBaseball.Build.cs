// WJSBaseball.Build.cs

using UnrealBuildTool;

public class WJSBaseball : ModuleRules
{
	public WJSBaseball(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			// Initial Dependencies
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",

			// UI
			"UMG", "Slate", "SlateCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PublicIncludePaths.AddRange(new string[] { "WJSBaseball" });
	}
}
