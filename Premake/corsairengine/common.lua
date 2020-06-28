-- Functionality that is common to all lua scripts in the Corsair Engine

function GetCopyLibraryPostBuildCommand(action)

	local postBuildCopyCommand = "{copy} %{cfg.buildtarget.abspath} ../../Binaries/%{prj.name}."..action..".%{cfg.buildcfg:lower()}".."%{cfg.buildtarget.extension}"

	if action == "vs2017" then
		postBuildCopyCommand = postBuildCopyCommand.."*"
	end
	
	return postBuildCopyCommand
end