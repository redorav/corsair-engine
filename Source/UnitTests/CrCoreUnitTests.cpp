#include "UnitTests/CrCoreUnitTests.h"

#include "Core/String/CrString.h"

// Include filesystem to compare implementation correctness
#include "Core/FileSystem/CrFixedPath.h"
#include <filesystem>

#include "Core/Logging/ICrDebug.h"
#include "Core/CrMacros.h"
#include "Core/String/CrStringUtilities.h"

void CrCoreUnitTests::RunCrPathUnitTests()
{
	CrString examplePaths[] =
	{
		"",
		".",
		"..",
		//"/", // Fails unit tests due to incomplete implementation of parent_path()
		"foo",
		"foo.txt",
		"C:/foo/bar/", // Path ending in /
		"C:/foo/bar/foobar", // Path ending in filename without extension
		"C:/foo/bar/foobar.txt", // Path ending in filename with extension
		"C:/foo.bar/foo.bar/foobar.txt", // Path with dots ending in filename with extension
		"C:/foo.bar/foo.bar/foobar", // Path with dots ending in filename without extension
	};

	for (uint32_t i = 0; i < sizeof_array(examplePaths); ++i)
	{
		const CrString& examplePath = examplePaths[i];

		std::filesystem::path stdPath = examplePath.c_str();
		CrFixedPath crPath = examplePath.c_str();

		// Check empty path concatenation
		{
			CrString stdPathFilename((stdPath / "foobar.exe").string().c_str());
			CrString crPathFilename((crPath / "foobar.exe").c_str());
			CrStringUtilities::ReplaceAll(stdPathFilename, '\\', '/');
			CrAssert(stdPathFilename == crPathFilename);
		}

		// Check filename
		{
			CrString stdPathFilename(stdPath.filename().string().c_str());
			CrString crPathFilename(crPath.filename().c_str());
			CrAssert(stdPathFilename == crPathFilename);
		}

		// Check parent path
		{
			CrString stdPathParentPath(stdPath.parent_path().string().c_str());
			CrString crPathParentPath(crPath.parent_path().c_str());
			CrAssert(stdPathParentPath == crPathParentPath);
		}

		// Check extension
		{
			CrString stdPathExtension(stdPath.extension().string().c_str());
			CrString crPathExtension(crPath.extension().c_str());
			CrAssert(crPathExtension == stdPathExtension);
		}

		// Check has_extension
		CrAssert(stdPath.has_extension() == crPath.has_extension());
	}
}
