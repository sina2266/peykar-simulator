#!/bin/bash

version=$1
developer_version=$2

IFS='.' read -ra version_array <<< "$version"
numeric_version=$(printf %02d "${version_array[@]}")
version_regex='[[:digit:]]*\.[[:digit:]]*\.[[:digit:]]*'

sed -i "s@    EXPECT_EQ(\"${version_regex}\", versionString);@    EXPECT_EQ(\"${version}\", versionString);@" tests/version/version.cpp
sed -i 's@    EXPECT_EQ(0x[[:digit:]]*U, version);@    EXPECT_EQ(0x'${numeric_version}'U, version);@' tests/version/version.cpp

sed -i 's/^set(_PROJECT_VERSION[^)]*)$/set(_PROJECT_VERSION '${version}')/' CMakeLists.txt
sed -i 's/^set(PROJECT_DEVELOPER_VERSION[^)]*)$/set(PROJECT_DEVELOPER_VERSION '${developer_version}')/' CMakeLists.txt

files=$(grep -rl "^LIBCELLML_VERSION = \"${version_regex}\"" .)
sed -i "s@LIBCELLML_VERSION = \"${version_regex}\"@LIBCELLML_VERSION = \"${version}\"@" $files

files=$(grep -rl "/\* The content of this file was generated using \(the\|a modified\) C profile of libCellML ${version_regex}\. \*/" *)
sed -i -E "s@/\* The content of this file was generated using (the|a modified) C profile of libCellML ${version_regex}\. \*/@/\* The content of this file was generated using \1 C profile of libCellML ${version}. \*/@" $files

files=$(grep -rl "# The content of this file was generated using \(the\|a modified\) Python profile of libCellML ${version_regex}\." *)
sed -i -E "s@# The content of this file was generated using (the|a modified) Python profile of libCellML ${version_regex}\.@# The content of this file was generated using \1 Python profile of libCellML ${version}.@" $files

files=$(grep -rl "const char LIBCELLML_VERSION\[\] = \"${version_regex}\";" *)
sed -i "s@const char LIBCELLML_VERSION\[\] = \"${version_regex}\";@const char LIBCELLML_VERSION\[\] = \"${version}\";@" $files

files=$(grep -rl "    expect(libcellml\.versionString()).toBe('${version_regex}');" *)
sed -i "s@    expect(libcellml\.versionString()).toBe('${version_regex}');@    expect(libcellml\.versionString()).toBe('${version}');@" $files

exit 0
