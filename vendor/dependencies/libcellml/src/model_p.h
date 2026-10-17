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

#pragma once

#include "libcellml/model.h"

#include "componententity_p.h"

namespace libcellml {

/**
 * @brief The Model::ModelImpl class.
 *
 * This class is the private implementation struct for the Model class.  Separating
 * the implementation from the definition allows for greater flexibility when
 * distributing the code.
 */
class Model::ModelImpl: public ComponentEntityImpl
{
public:
    std::vector<UnitsPtr> mUnits;

    std::vector<UnitsPtr>::const_iterator findUnits(const std::string &name) const;
    std::vector<UnitsPtr>::const_iterator findUnits(const UnitsPtr &units) const;

    /**
     * @brief Test to see if the units of this @ref Model are equal to the units of the @p other @ref Model.
     *
     * The units of this @ref Model are tested against the @p other @ref Model's units, and compared for equality.
     *
     * @param other The @ref Model to test against.
     *
     * @return @c true if this @ref Model's units are equal to the @p other @ref Model's units, @c false otherwise.
     */
    bool equalUnits(const ModelPtr &other) const;
};

} // namespace libcellml
