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

#include <string>

#include "libcellml/logger.h"
#include "libcellml/strict.h"
#include "libcellml/types.h"

namespace libcellml {

/**
 * @brief The Importer class.
 *
 * The Importer class is for representing a CellML Importer.
 */
class LIBCELLML_EXPORT Importer: public Logger, public Strict
{
public:
    ~Importer() override; /**< Destructor, @private. */
    Importer(const Importer &rhs) = delete; /**< Copy constructor, @private. */
    Importer(Importer &&rhs) noexcept = delete; /**< Move constructor, @private. */
    Importer &operator=(Importer rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create an @c Importer object.
     *
     * Factory method to create an @c Importer.  Create a
     * strict importer with:
     *
     * @code
     *   auto importer = libcellml::Importer::create();
     * @endcode
     *
     * Create an importer with the strict flag set to @c false with:
     *
     * @code
     *   ImporterPtr importer = libcellml::Importer::create(false);
     * @endcode
     *
     * @param strict [optional] A boolean value to set the strict flag to.
     *
     * @return A smart pointer to an @c Importer object.
     */
    static ImporterPtr create(bool strict = true) noexcept;

    /**
     * @brief Flatten the @p model.
     *
     * Instantiate all imports and removes them from the @p model.
     * The result is a self-contained model requiring no external
     * resources and having no imports.
     *
     * All existing issues will be removed before the model is flattened.
     *
     * @sa clone
     *
     * @param model A @c ModelPtr whose imports will be resolved.
     *
     * @return If the operation is successful, a new @c ModelPtr to the flattened model; otherwise, the @c nullptr.
     */
    ModelPtr flattenModel(const ModelPtr &model);

    /**
     * @brief Resolve all imports in the @p model.
     *
     * Resolve all @c Component and @c Units imports by loading the models
     * from local disk through relative URLs.  The @p basePath defines
     * the full path to the directory that import URLs are relative to.
     *
     * All existing issues will be removed before any imports are resolved.
     *
     * @param model The @c Model whose imports need resolution.
     * @param basePath The full path used to resolve relative import URLs.
     *
     * @return @c true if all imports have been resolved successfully, @c false otherwise.
     */
    bool resolveImports(ModelPtr &model, const std::string &basePath);

    /**
     * @brief Return the number of models present in the importer's library.
     *
     * @return The number of models present in the importer's library.
     */
    size_t libraryCount();

    /**
     * @brief Retrieve the @c ModelPtr instance from the importer library which was loaded
     *        from the given @p key.
     *
     * @return A @c ModelPtr instance.
     */
    ModelPtr library(const std::string &key);

    /**
     * @brief Get a model from the library at the given @p index.
     *
     * Return a reference to the model at the index @p index for this
     * importer's library. If the index is not valid a @c nullptr is returned. The valid
     * range for the index is [0, \#library items).
     *
     * @param index The index of the model to return.
     *
     * @return A reference to the model at the given index on success, @c nullptr otherwise.
     */
    ModelPtr library(const size_t &index);

    /**
     * @brief Get the key string under which a model is stored in the library, at the given @p index.
     *
     * Get the key string under which a model is stored in the library, at the given @p index.
     *
     * @param index The index of the key to return.
     *
     * @return If successful, a string under which the model has been stored, or an empty string otherwise.
     */
    std::string key(const size_t &index);

    /**
     * @brief Manually add a local @c ModelPtr model instance to the importer library,
     *        using the given @p key as a reference.
     *
     * If the given key already exists in the library, the function will return false
     * and the library will not be changed.
     *
     * @sa replaceModel
     *
     * @param model a @c ModelPtr instance to add.
     * @param key a @c std::string representing the key to associate with the model.
     *
     * @return @c true if the model was added, @c false if it was not.
     */
    bool addModel(const ModelPtr &model, const std::string &key);

    /**
     * @brief Replace a @c ModelPtr model instance in the importer library,
     *        using the given @p key as a reference.
     *
     * If the given key already exists in the library, the function will replace its
     * model with the one supplied, and return @c true.
     *
     * If the given key does not exist, the function will return @c false, and
     * the library will be unchanged.
     *
     * @sa addModel.
     *
     * @param model a @c ModelPtr instance to replace the current one.
     * @param key a @c std::string representing the key at which to replace the model.
     *
     * @return @c true if the model was replaced, @c false if it was not.
     */
    bool replaceModel(const ModelPtr &model, const std::string &key);

    /**
     * @brief Clear the links with other models from all import sources.
     *
     * Clear the links with other models from all import sources.
     */
    void clearImports(ModelPtr &model);

    /**
     * @brief Remove all models from the library.
     *
     * Remove all models from the library.
     */
    void removeAllModels();

    /**
     * @brief Add an import source item to this importer.
     *
     * Add import source by reference to the importer.
     *
     * The function will return @c false and no action is taken if:
     *  - The @p importSource pointer already exists in this importer; or
     *  - The @p importSource is @c nullptr.
     *
     * @param importSource The import source to add.
     *
     * @return @c true if the import source item is added or @c false otherwise.
     */
    bool addImportSource(const ImportSourcePtr &importSource);

    /**
     * @brief Get the number of import source items in the importer.
     *
     * Returns the number of import source items the importer contains.
     *
     * @return The number of import source items.
     */
    size_t importSourceCount() const;

    /**
     * @brief Get the import source item at the given @p index.
     *
     * Returns a reference to an import source at the given @p index.  If the @p index
     * is not valid a @c nullptr is returned, the range of valid values for the
     * index is [0, \#importSources).
     *
     * @param index The index of the import source to return.
     *
     * @return A reference to the import source at the given @p index on success, @c nullptr otherwise.
     */
    ImportSourcePtr importSource(size_t index) const;

    /**
     * @brief Remove the import source at the given @p index.
     *
     * Remove the import source from this importer at the given @p index.
     * @p index must be in the range [0, \#importSources).
     *
     * @param index The index of the import source to remove.
     *
     * @return @c true if the import source was removed, @c false otherwise.
     */
    bool removeImportSource(size_t index);

    /**
     * @overload
     *
     * @brief Remove the import source at the given reference.
     *
     * @param importSource The pointer to the import source to remove.
     *
     * @return @c true if the import source was removed, @c false otherwise.
     */
    bool removeImportSource(const ImportSourcePtr &importSource);

    /**
     * @brief Remove all import sources stored in this importer.
     *
     * Clears all import sources that have been added to this importer.
     */
    bool removeAllImportSources();

    /**
     * @brief Tests to see if the import source is within this importer.
     *
     * Tests to see if the given import source is contained within this importer.
     * Returns @c true if the import source is in the importer and @c false otherwise.
     *
     * @param importSource The import source to test for existence in this importer.
     *
     * @return @c true if the import source is in the importer and @c false otherwise.
     */
    bool hasImportSource(const ImportSourcePtr &importSource) const;

#ifdef JAVASCRIPT_BINDINGS
#    include "strict.impl"
#endif

private:
    Importer(); /**< Constructor, @private. */
    explicit Importer(const std::string &name); /**< Constructor with std::string parameter, @private. */

    class ImporterImpl; /**< Forward declaration for pImpl idiom, @private. */

    ImporterImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
    const ImporterImpl *pFunc() const; /**< Const getter for private implementation pointer, @private. */
};

} // namespace libcellml
