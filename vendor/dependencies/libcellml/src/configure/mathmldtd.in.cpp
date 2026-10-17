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

#include "mathmldtd.h"

#include <array>

namespace libcellml {

static const std::array<unsigned char, ${COMPRESSED_MATHML_DTD_LEN}> COMPRESSED_MATHML_DTD = ${COMPRESSED_MATHML_DTD_DATA};

const unsigned char *compressedMathMLDTD()
{
    return &COMPRESSED_MATHML_DTD[0];
}

} // namespace libcellml
