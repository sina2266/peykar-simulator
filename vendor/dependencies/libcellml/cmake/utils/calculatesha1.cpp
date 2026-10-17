/*
Copyright libCellML Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <fstream>
#include <string>

#include <libcellml/generatorprofile.h>

#include "generatorprofiletools.h"
#include "utilities.h"

int main()
{
    auto cGeneratorProfile = libcellml::GeneratorProfile::create();
    auto pyGeneratorProfile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    auto cGeneratorProfileRepr = libcellml::generatorProfileAsString(cGeneratorProfile);
    std::string cSha1Value = libcellml::sha1(cGeneratorProfileRepr);

    auto pyGeneratorProfileRepr = libcellml::generatorProfileAsString(pyGeneratorProfile);
    std::string pySha1Value = libcellml::sha1(pyGeneratorProfileRepr);

    std::ofstream outFile("generatorprofilesha1values.cmake");

    outFile << "set(C_GENERATOR_PROFILE_SHA1_VALUE " << cSha1Value << ")" << std::endl;
    outFile << "set(PYTHON_GENERATOR_PROFILE_SHA1_VALUE " << pySha1Value << ")" << std::endl;

    outFile.close();

    return 0;
};
