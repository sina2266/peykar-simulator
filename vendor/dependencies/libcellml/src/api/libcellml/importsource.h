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

#include "libcellml/entity.h"
#include "libcellml/exportdefinitions.h"

#ifndef SWIG
template class LIBCELLML_EXPORT std::weak_ptr<libcellml::ImportSource>;
#endif

namespace libcellml {

/**
 * @brief The ImportSource class.
 *
 * The import source class is for defining a location at which a CellML model
 * is expected to be obtainable (the `source`). The import source also acts as
 * a handle to indicate that imported entities originate from the same instance of
 * that imported source.
 */
class LIBCELLML_EXPORT ImportSource: public Entity
#ifndef SWIG
    ,
                                     public std::enable_shared_from_this<ImportSource>
#endif
{
public:
    ~ImportSource() override; /**< Destructor, @private. */
    ImportSource(const ImportSource &rhs) = delete; /**< Copy constructor, @private. */
    ImportSource(ImportSource &&rhs) noexcept = delete; /**< Move constructor, @private. */
    ImportSource &operator=(ImportSource rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a @c ImportSource object.
     *
     * Factory method to create an @c ImportSource.  Create an
     * import source with::
     *
     *   ImportSourcePtr importSource = libcellml::ImportSource::create();
     *
     * @return A smart pointer to an @c ImportSource object.
     */
    static ImportSourcePtr create() noexcept;

    /**
     * @brief Get the source @ref Model's URL.
     *
     * Get the source @ref Model's URL set in this instance. If no source @ref Model
     * URL is set then return an empty string.
     *
     * @return The URL of the source @ref Model if set otherwise the empty string.
     */
    std::string url() const;

    /**
     * @brief Set the source @ref Model's URL.
     *
     * Set the source @ref Model's URL that this @c ImportSource refers to.
     *
     * @param url The source @ref Model's URL.
     */
    void setUrl(const std::string &url);

    /**
     * @brief Get the @ref Model that resolves the import.
     *
     * Get the @ref Model which has been assigned to resolve this @c ImportSource. If no @ref Model
     * has been assigned then return the @c nullptr.
     *
     * @return The @ref Model used to resolve this @c ImportSource.
     */
    ModelPtr model() const;

    /**
     * @brief Provide the @ref Model used to resolve this import.
     *
     * Uses the provided @ref Model to resolve this @c ImportSource, which should correspond
     * to the @c ImportSource identified by this import.
     *
     * @param model The @ref Model to use in resolving this @c ImportSource.
     */
    void setModel(const ModelPtr &model);

    /**
     * @brief Remove the model from this import source.
     *
     * Remove the reference to a model from this import source.
     */
    void removeModel();

    /**
     * @brief Test if this @c ImportSource is resolved.
     *
     * Method to test if this @c ImportSource has been resolved, i.e. the source @ref Model has
     * been assigned. Returns @c true if the @c ImportSource is resolved otherwise returns
     * @c false.
     *
     * @return @c true if the @c ImportSource has been resolved, @c false otherwise.
     */
    bool hasModel() const;

    /**
     * @brief Create a clone of this import source.
     *
     * Creates a full separate copy of this import source without copying
     * the parent.
     *
     * @return a new @c ImportSourcePtr to the cloned import source.
     */
    ImportSourcePtr clone() const;

protected:
    bool doEquals(const EntityPtr &other) const override; /**< Virtual implementation method for equals, @private. */

private:
    ImportSource(); /**< Constructor, @private. */

    class ImportSourceImpl; /**< Forward declaration for pImpl idiom, @private. */

    ImportSourceImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
    const ImportSourceImpl *pFunc() const; /**< Const getter for private implementation pointer, @private. */
};

} // namespace libcellml
