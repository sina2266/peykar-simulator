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

#include "generatorprofiletools.h"

#include <array>
#include <iomanip>
#include <sstream>

#include "libcellml/generatorprofile.h"

namespace libcellml {

// The below code is used to compute the SHA-1 value of a string, based on the
// code available at https://github.com/vog/sha1.

static const size_t BLOCK_INTS = 16;
static const size_t BLOCK_BYTES = BLOCK_INTS * 4;

static void bufferToBlock(const std::string &buffer,
                          std::array<uint32_t, BLOCK_INTS> &block)
{
    for (size_t i = 0; i < BLOCK_INTS; ++i) {
        block.at(i) = (uint32_t(buffer[4 * i + 3]) & 0xffU)
                      | (uint32_t(buffer[4 * i + 2]) & 0xffU) << 8U
                      | (uint32_t(buffer[4 * i + 1]) & 0xffU) << 16U
                      | (uint32_t(buffer[4 * i + 0]) & 0xffU) << 24U;
    }
}

static uint32_t rol(const uint32_t value, const size_t bits)
{
    return (value << bits) | (value >> (32 - bits));
}

static uint32_t blk(const std::array<uint32_t, BLOCK_INTS> block,
                    const size_t i)
{
    return rol(block.at((i + 13) & 15U) ^ block.at((i + 8) & 15U) ^ block.at((i + 2) & 15U) ^ block.at(i), 1);
}

static void r0(const std::array<uint32_t, BLOCK_INTS> block, const uint32_t v,
               uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z,
               const size_t i)
{
    z += ((w & (x ^ y)) ^ y) + block.at(i) + 0x5a827999U + rol(v, 5);
    w = rol(w, 30);
}

static void r1(std::array<uint32_t, BLOCK_INTS> &block, const uint32_t v,
               uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z,
               const size_t i)
{
    block.at(i) = blk(block, i);

    z += ((w & (x ^ y)) ^ y) + block.at(i) + 0x5a827999U + rol(v, 5);
    w = rol(w, 30);
}

static void r2(std::array<uint32_t, BLOCK_INTS> &block, const uint32_t v,
               uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z,
               const size_t i)
{
    block.at(i) = blk(block, i);

    z += (w ^ x ^ y) + block.at(i) + 0x6ed9eba1U + rol(v, 5);
    w = rol(w, 30);
}

static void r3(std::array<uint32_t, BLOCK_INTS> &block, const uint32_t v,
               uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z,
               const size_t i)
{
    block.at(i) = blk(block, i);

    z += (((w | x) & y) | (w & x)) + block.at(i) + 0x8f1bbcdcU + rol(v, 5);
    w = rol(w, 30);
}

static void r4(std::array<uint32_t, BLOCK_INTS> &block, const uint32_t v,
               uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z,
               const size_t i)
{
    block.at(i) = blk(block, i);

    z += (w ^ x ^ y) + block.at(i) + 0xca62c1d6U + rol(v, 5);
    w = rol(w, 30);
}

static void transform(std::array<uint32_t, 5> &digest,
                      std::array<uint32_t, BLOCK_INTS> &block,
                      uint64_t &transforms)
{
    uint32_t a = digest[0];
    uint32_t b = digest[1];
    uint32_t c = digest[2];
    uint32_t d = digest[3];
    uint32_t e = digest[4];

    r0(block, a, b, c, d, e, 0);
    r0(block, e, a, b, c, d, 1);
    r0(block, d, e, a, b, c, 2);
    r0(block, c, d, e, a, b, 3);
    r0(block, b, c, d, e, a, 4);
    r0(block, a, b, c, d, e, 5);
    r0(block, e, a, b, c, d, 6);
    r0(block, d, e, a, b, c, 7);
    r0(block, c, d, e, a, b, 8);
    r0(block, b, c, d, e, a, 9);
    r0(block, a, b, c, d, e, 10);
    r0(block, e, a, b, c, d, 11);
    r0(block, d, e, a, b, c, 12);
    r0(block, c, d, e, a, b, 13);
    r0(block, b, c, d, e, a, 14);
    r0(block, a, b, c, d, e, 15);
    r1(block, e, a, b, c, d, 0);
    r1(block, d, e, a, b, c, 1);
    r1(block, c, d, e, a, b, 2);
    r1(block, b, c, d, e, a, 3);
    r2(block, a, b, c, d, e, 4);
    r2(block, e, a, b, c, d, 5);
    r2(block, d, e, a, b, c, 6);
    r2(block, c, d, e, a, b, 7);
    r2(block, b, c, d, e, a, 8);
    r2(block, a, b, c, d, e, 9);
    r2(block, e, a, b, c, d, 10);
    r2(block, d, e, a, b, c, 11);
    r2(block, c, d, e, a, b, 12);
    r2(block, b, c, d, e, a, 13);
    r2(block, a, b, c, d, e, 14);
    r2(block, e, a, b, c, d, 15);
    r2(block, d, e, a, b, c, 0);
    r2(block, c, d, e, a, b, 1);
    r2(block, b, c, d, e, a, 2);
    r2(block, a, b, c, d, e, 3);
    r2(block, e, a, b, c, d, 4);
    r2(block, d, e, a, b, c, 5);
    r2(block, c, d, e, a, b, 6);
    r2(block, b, c, d, e, a, 7);
    r3(block, a, b, c, d, e, 8);
    r3(block, e, a, b, c, d, 9);
    r3(block, d, e, a, b, c, 10);
    r3(block, c, d, e, a, b, 11);
    r3(block, b, c, d, e, a, 12);
    r3(block, a, b, c, d, e, 13);
    r3(block, e, a, b, c, d, 14);
    r3(block, d, e, a, b, c, 15);
    r3(block, c, d, e, a, b, 0);
    r3(block, b, c, d, e, a, 1);
    r3(block, a, b, c, d, e, 2);
    r3(block, e, a, b, c, d, 3);
    r3(block, d, e, a, b, c, 4);
    r3(block, c, d, e, a, b, 5);
    r3(block, b, c, d, e, a, 6);
    r3(block, a, b, c, d, e, 7);
    r3(block, e, a, b, c, d, 8);
    r3(block, d, e, a, b, c, 9);
    r3(block, c, d, e, a, b, 10);
    r3(block, b, c, d, e, a, 11);
    r4(block, a, b, c, d, e, 12);
    r4(block, e, a, b, c, d, 13);
    r4(block, d, e, a, b, c, 14);
    r4(block, c, d, e, a, b, 15);
    r4(block, b, c, d, e, a, 0);
    r4(block, a, b, c, d, e, 1);
    r4(block, e, a, b, c, d, 2);
    r4(block, d, e, a, b, c, 3);
    r4(block, c, d, e, a, b, 4);
    r4(block, b, c, d, e, a, 5);
    r4(block, a, b, c, d, e, 6);
    r4(block, e, a, b, c, d, 7);
    r4(block, d, e, a, b, c, 8);
    r4(block, c, d, e, a, b, 9);
    r4(block, b, c, d, e, a, 10);
    r4(block, a, b, c, d, e, 11);
    r4(block, e, a, b, c, d, 12);
    r4(block, d, e, a, b, c, 13);
    r4(block, c, d, e, a, b, 14);
    r4(block, b, c, d, e, a, 15);

    digest[0] += a;
    digest[1] += b;
    digest[2] += c;
    digest[3] += d;
    digest[4] += e;

    ++transforms;
}

std::string sha1(const std::string &string)
{
    std::array<uint32_t, 5> digest = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
    std::string buffer;
    uint64_t transforms = 0;
    std::istringstream is(string);

    while (true) {
        std::array<char, BLOCK_BYTES> sbuf = {};

        is.read(sbuf.data(), int64_t(BLOCK_BYTES - buffer.size()));

        buffer.append(sbuf.data(), size_t(is.gcount()));

        if (buffer.size() != BLOCK_BYTES) {
            break;
        }

        std::array<uint32_t, BLOCK_INTS> block = {};

        bufferToBlock(buffer, block);
        transform(digest, block, transforms);

        buffer.clear();
    }

    uint64_t totalBits = (transforms * BLOCK_BYTES + buffer.size()) * 8;

    buffer += "\x80";

    size_t origSize = buffer.size();

    while (buffer.size() < BLOCK_BYTES) {
        buffer += char(0x00);
    }

    std::array<uint32_t, BLOCK_INTS> block = {};

    bufferToBlock(buffer, block);

    if (origSize > BLOCK_BYTES - 8) {
        transform(digest, block, transforms);

        for (size_t i = 0; i < BLOCK_INTS - 2; ++i) {
            block.at(i) = 0;
        }
    }

    block[BLOCK_INTS - 1] = uint32_t(totalBits);
    block[BLOCK_INTS - 2] = uint32_t(totalBits >> 32U);

    transform(digest, block, transforms);

    std::ostringstream result;

    for (uint32_t d : digest) {
        result << std::hex << std::setfill('0') << std::setw(8);
        result << d;
    }

    return result.str();
}

std::string generatorProfileAsString(const GeneratorProfilePtr &generatorProfile)
{
    // Whether the profile requires an interface to be generated.

    static const std::string TRUE_VALUE = "true";
    static const std::string FALSE_VALUE = "false";

    auto profileContents = generatorProfile->hasInterface() ?
                               TRUE_VALUE :
                               FALSE_VALUE;

    // Equality.

    profileContents += generatorProfile->equalityString();

    // Relational and logical operators.

    profileContents += generatorProfile->eqString()
                       + generatorProfile->neqString()
                       + generatorProfile->ltString()
                       + generatorProfile->leqString()
                       + generatorProfile->gtString()
                       + generatorProfile->geqString()
                       + generatorProfile->andString()
                       + generatorProfile->orString()
                       + generatorProfile->xorString()
                       + generatorProfile->notString();

    profileContents += (generatorProfile->hasEqOperator() ?
                            TRUE_VALUE :
                            FALSE_VALUE)
                       + (generatorProfile->hasNeqOperator() ?
                              TRUE_VALUE :
                              FALSE_VALUE)
                       + (generatorProfile->hasLtOperator() ?
                              TRUE_VALUE :
                              FALSE_VALUE)
                       + (generatorProfile->hasLeqOperator() ?
                              TRUE_VALUE :
                              FALSE_VALUE)
                       + (generatorProfile->hasGtOperator() ?
                              TRUE_VALUE :
                              FALSE_VALUE)
                       + (generatorProfile->hasGeqOperator() ?
                              TRUE_VALUE :
                              FALSE_VALUE)
                       + (generatorProfile->hasAndOperator() ?
                              TRUE_VALUE :
                              FALSE_VALUE)
                       + (generatorProfile->hasOrOperator() ?
                              TRUE_VALUE :
                              FALSE_VALUE)
                       + (generatorProfile->hasXorOperator() ?
                              TRUE_VALUE :
                              FALSE_VALUE)
                       + (generatorProfile->hasNotOperator() ?
                              TRUE_VALUE :
                              FALSE_VALUE);

    // Arithmetic operators.

    profileContents += generatorProfile->plusString()
                       + generatorProfile->minusString()
                       + generatorProfile->timesString()
                       + generatorProfile->divideString()
                       + generatorProfile->powerString()
                       + generatorProfile->squareRootString()
                       + generatorProfile->squareString()
                       + generatorProfile->absoluteValueString()
                       + generatorProfile->exponentialString()
                       + generatorProfile->naturalLogarithmString()
                       + generatorProfile->commonLogarithmString()
                       + generatorProfile->ceilingString()
                       + generatorProfile->floorString()
                       + generatorProfile->minString()
                       + generatorProfile->maxString()
                       + generatorProfile->remString();

    profileContents += generatorProfile->hasPowerOperator() ?
                           TRUE_VALUE :
                           FALSE_VALUE;

    // Trigonometric operators.

    profileContents += generatorProfile->sinString()
                       + generatorProfile->cosString()
                       + generatorProfile->tanString()
                       + generatorProfile->secString()
                       + generatorProfile->cscString()
                       + generatorProfile->cotString()
                       + generatorProfile->sinhString()
                       + generatorProfile->coshString()
                       + generatorProfile->tanhString()
                       + generatorProfile->sechString()
                       + generatorProfile->cschString()
                       + generatorProfile->cothString()
                       + generatorProfile->asinString()
                       + generatorProfile->acosString()
                       + generatorProfile->atanString()
                       + generatorProfile->asecString()
                       + generatorProfile->acscString()
                       + generatorProfile->acotString()
                       + generatorProfile->asinhString()
                       + generatorProfile->acoshString()
                       + generatorProfile->atanhString()
                       + generatorProfile->asechString()
                       + generatorProfile->acschString()
                       + generatorProfile->acothString();

    // Piecewise statement.

    profileContents += generatorProfile->conditionalOperatorIfString()
                       + generatorProfile->conditionalOperatorElseString()
                       + generatorProfile->piecewiseIfString()
                       + generatorProfile->piecewiseElseString();

    profileContents += generatorProfile->hasConditionalOperator() ?
                           TRUE_VALUE :
                           FALSE_VALUE;

    // Constants.

    profileContents += generatorProfile->trueString()
                       + generatorProfile->falseString()
                       + generatorProfile->eString()
                       + generatorProfile->piString()
                       + generatorProfile->infString()
                       + generatorProfile->nanString();

    // Arithmetic functions.

    profileContents += generatorProfile->eqFunctionString()
                       + generatorProfile->neqFunctionString()
                       + generatorProfile->ltFunctionString()
                       + generatorProfile->leqFunctionString()
                       + generatorProfile->gtFunctionString()
                       + generatorProfile->geqFunctionString()
                       + generatorProfile->andFunctionString()
                       + generatorProfile->orFunctionString()
                       + generatorProfile->xorFunctionString()
                       + generatorProfile->notFunctionString()
                       + generatorProfile->minFunctionString()
                       + generatorProfile->maxFunctionString();

    // Trigonometric functions.

    profileContents += generatorProfile->secFunctionString()
                       + generatorProfile->cscFunctionString()
                       + generatorProfile->cotFunctionString()
                       + generatorProfile->sechFunctionString()
                       + generatorProfile->cschFunctionString()
                       + generatorProfile->cothFunctionString()
                       + generatorProfile->asecFunctionString()
                       + generatorProfile->acscFunctionString()
                       + generatorProfile->acotFunctionString()
                       + generatorProfile->asechFunctionString()
                       + generatorProfile->acschFunctionString()
                       + generatorProfile->acothFunctionString();

    // Miscellaneous.
    // Note: we do NOT include interfaceFileNameString() since it may be the
    //       only thing that someone might change, so that the generated file
    //       works with the file name it is to be given.

    profileContents += generatorProfile->commentString()
                       + generatorProfile->originCommentString();

    profileContents += generatorProfile->interfaceHeaderString()
                       + generatorProfile->implementationHeaderString();

    profileContents += generatorProfile->interfaceVersionString()
                       + generatorProfile->implementationVersionString();

    profileContents += generatorProfile->interfaceLibcellmlVersionString()
                       + generatorProfile->implementationLibcellmlVersionString();

    profileContents += generatorProfile->interfaceStateCountString()
                       + generatorProfile->implementationStateCountString();

    profileContents += generatorProfile->interfaceConstantCountString()
                       + generatorProfile->implementationConstantCountString();

    profileContents += generatorProfile->interfaceComputedConstantCountString()
                       + generatorProfile->implementationComputedConstantCountString();

    profileContents += generatorProfile->interfaceAlgebraicVariableCountString()
                       + generatorProfile->implementationAlgebraicVariableCountString();

    profileContents += generatorProfile->interfaceExternalVariableCountString()
                       + generatorProfile->implementationExternalVariableCountString();

    profileContents += generatorProfile->variableInfoObjectString();

    profileContents += generatorProfile->interfaceVoiInfoString()
                       + generatorProfile->implementationVoiInfoString();

    profileContents += generatorProfile->interfaceStateInfoString()
                       + generatorProfile->implementationStateInfoString();

    profileContents += generatorProfile->interfaceConstantInfoString()
                       + generatorProfile->implementationConstantInfoString();

    profileContents += generatorProfile->interfaceComputedConstantInfoString()
                       + generatorProfile->implementationComputedConstantInfoString();

    profileContents += generatorProfile->interfaceAlgebraicVariableInfoString()
                       + generatorProfile->implementationAlgebraicVariableInfoString();

    profileContents += generatorProfile->interfaceExternalVariableInfoString()
                       + generatorProfile->implementationExternalVariableInfoString();

    profileContents += generatorProfile->variableInfoEntryString();

    profileContents += generatorProfile->voiString();

    profileContents += generatorProfile->statesArrayString()
                       + generatorProfile->ratesArrayString()
                       + generatorProfile->constantsArrayString()
                       + generatorProfile->computedConstantsArrayString()
                       + generatorProfile->algebraicVariablesArrayString()
                       + generatorProfile->externalVariablesArrayString();

    profileContents += generatorProfile->externalVariableMethodTypeDefinitionString(false)
                       + generatorProfile->externalVariableMethodTypeDefinitionString(true);

    profileContents += generatorProfile->externalVariableMethodCallString(false)
                       + generatorProfile->externalVariableMethodCallString(true);

    profileContents += generatorProfile->rootFindingInfoObjectString(false, false)
                       + generatorProfile->rootFindingInfoObjectString(false, true)
                       + generatorProfile->rootFindingInfoObjectString(true, false)
                       + generatorProfile->rootFindingInfoObjectString(true, true)
                       + generatorProfile->externNlaSolveMethodString()
                       + generatorProfile->findRootCallString(false, false)
                       + generatorProfile->findRootCallString(false, true)
                       + generatorProfile->findRootCallString(true, false)
                       + generatorProfile->findRootCallString(true, true)
                       + generatorProfile->findRootMethodString(false, false)
                       + generatorProfile->findRootMethodString(false, true)
                       + generatorProfile->findRootMethodString(true, false)
                       + generatorProfile->findRootMethodString(true, true)
                       + generatorProfile->nlaSolveCallString(false, false)
                       + generatorProfile->nlaSolveCallString(false, true)
                       + generatorProfile->nlaSolveCallString(true, false)
                       + generatorProfile->nlaSolveCallString(true, true)
                       + generatorProfile->objectiveFunctionMethodString(false, false)
                       + generatorProfile->objectiveFunctionMethodString(false, true)
                       + generatorProfile->objectiveFunctionMethodString(true, false)
                       + generatorProfile->objectiveFunctionMethodString(true, true)
                       + generatorProfile->uArrayString()
                       + generatorProfile->fArrayString();

    profileContents += generatorProfile->interfaceCreateStatesArrayMethodString()
                       + generatorProfile->implementationCreateStatesArrayMethodString();

    profileContents += generatorProfile->interfaceCreateConstantsArrayMethodString()
                       + generatorProfile->implementationCreateConstantsArrayMethodString();

    profileContents += generatorProfile->interfaceCreateComputedConstantsArrayMethodString()
                       + generatorProfile->implementationCreateComputedConstantsArrayMethodString();

    profileContents += generatorProfile->interfaceCreateAlgebraicVariablesArrayMethodString()
                       + generatorProfile->implementationCreateAlgebraicVariablesArrayMethodString();

    profileContents += generatorProfile->interfaceCreateExternalVariablesArrayMethodString()
                       + generatorProfile->implementationCreateExternalVariablesArrayMethodString();

    profileContents += generatorProfile->interfaceDeleteArrayMethodString()
                       + generatorProfile->implementationDeleteArrayMethodString();

    profileContents += generatorProfile->interfaceInitialiseArraysMethodString(false)
                       + generatorProfile->implementationInitialiseArraysMethodString(false);

    profileContents += generatorProfile->interfaceInitialiseArraysMethodString(true)
                       + generatorProfile->implementationInitialiseArraysMethodString(true);

    profileContents += generatorProfile->interfaceComputeComputedConstantsMethodString(false)
                       + generatorProfile->implementationComputeComputedConstantsMethodString(false);

    profileContents += generatorProfile->interfaceComputeComputedConstantsMethodString(true)
                       + generatorProfile->implementationComputeComputedConstantsMethodString(true);

    profileContents += generatorProfile->interfaceComputeRatesMethodString(false)
                       + generatorProfile->implementationComputeRatesMethodString(false);

    profileContents += generatorProfile->interfaceComputeRatesMethodString(true)
                       + generatorProfile->implementationComputeRatesMethodString(true);

    profileContents += generatorProfile->interfaceComputeVariablesMethodString(false, false)
                       + generatorProfile->implementationComputeVariablesMethodString(false, false);

    profileContents += generatorProfile->interfaceComputeVariablesMethodString(false, true)
                       + generatorProfile->implementationComputeVariablesMethodString(false, true);

    profileContents += generatorProfile->interfaceComputeVariablesMethodString(true, false)
                       + generatorProfile->implementationComputeVariablesMethodString(true, false);

    profileContents += generatorProfile->interfaceComputeVariablesMethodString(true, true)
                       + generatorProfile->implementationComputeVariablesMethodString(true, true);

    profileContents += generatorProfile->emptyMethodString();

    profileContents += generatorProfile->indentString();

    profileContents += generatorProfile->variableDeclarationString();

    profileContents += generatorProfile->openArrayString()
                       + generatorProfile->closeArrayString();

    profileContents += generatorProfile->arrayElementSeparatorString();

    profileContents += generatorProfile->commandSeparatorString();

    return profileContents;
}

} // namespace libcellml
