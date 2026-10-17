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

#include "libcellml/strict.h"

namespace libcellml {

/**
 * @brief The Strict::StrictImpl class.
 *
 * The private implementation for the Strict class.
 */
class Strict::StrictImpl
{
public:
    bool mStrict = true;
};

Strict::Strict()
    : mPimpl(new StrictImpl())
{
}

Strict::~Strict()
{
    delete mPimpl;
}

void Strict::setStrict(bool strict)
{
    pFunc()->mStrict = strict;
}

bool Strict::isStrict() const
{
    return pFunc()->mStrict;
}

} // namespace libcellml
