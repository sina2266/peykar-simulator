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

const libCellMLModule = require('libcellml.js/libcellml.common')
let libcellml = null

describe("GeneratorProfile tests", () => {
  beforeAll(async () => {
    libcellml = await libCellMLModule();
  });
  test("Checking GeneratorProfile.profile.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)
    expect(x.profile()).toBe(libcellml.GeneratorProfile.Profile.C)
    expect(libcellml.GeneratorProfile.profileAsString(x.profile())).toBe("c")

    x.setProfile(libcellml.GeneratorProfile.Profile.PYTHON)
    expect(x.profile()).toBe(libcellml.GeneratorProfile.Profile.PYTHON)
    expect(libcellml.GeneratorProfile.profileAsString(x.profile())).toBe("python")
  });
  test("Checking GeneratorProfile.hasInterface.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasInterface(true)
    expect(x.hasInterface()).toBe(true)
  });
  test("Checking GeneratorProfile.equalityString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setEqualityString("something")
    expect(x.equalityString()).toBe("something")
  });
  test("Checking GeneratorProfile.eqString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setEqString("something")
    expect(x.eqString()).toBe("something")
  });
  test("Checking GeneratorProfile.neqString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setNeqString("something")
    expect(x.neqString()).toBe("something")
  });
  test("Checking GeneratorProfile.ltString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setLtString("something")
    expect(x.ltString()).toBe("something")
  });
  test("Checking GeneratorProfile.leqString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setLeqString("something")
    expect(x.leqString()).toBe("something")
  });
  test("Checking GeneratorProfile.gtString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setGtString("something")
    expect(x.gtString()).toBe("something")
  });
  test("Checking GeneratorProfile.geqString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setGeqString("something")
    expect(x.geqString()).toBe("something")
  });
  test("Checking GeneratorProfile.andString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAndString("something")
    expect(x.andString()).toBe("something")
  });
  test("Checking GeneratorProfile.orString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setOrString("something")
    expect(x.orString()).toBe("something")
  });
  test("Checking GeneratorProfile.xorString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setXorString("something")
    expect(x.xorString()).toBe("something")
  });
  test("Checking GeneratorProfile.notString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setNotString("something")
    expect(x.notString()).toBe("something")
  });
  test("Checking GeneratorProfile.hasEqOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasEqOperator(true)
    expect(x.hasEqOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.hasNeqOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasNeqOperator(true)
    expect(x.hasNeqOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.hasLtOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasLtOperator(true)
    expect(x.hasLtOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.hasLeqOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasLeqOperator(true)
    expect(x.hasLeqOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.hasGtOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasGtOperator(true)
    expect(x.hasGtOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.hasGeqOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasGeqOperator(true)
    expect(x.hasGeqOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.hasAndOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasAndOperator(true)
    expect(x.hasAndOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.hasOrOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasOrOperator(true)
    expect(x.hasOrOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.hasXorOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasXorOperator(true)
    expect(x.hasXorOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.hasNotOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasNotOperator(true)
    expect(x.hasNotOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.plusString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setPlusString("something")
    expect(x.plusString()).toBe("something")
  });
  test("Checking GeneratorProfile.minusString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setMinusString("something")
    expect(x.minusString()).toBe("something")
  });
  test("Checking GeneratorProfile.timesString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setTimesString("something")
    expect(x.timesString()).toBe("something")
  });
  test("Checking GeneratorProfile.divideString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setDivideString("something")
    expect(x.divideString()).toBe("something")
  });
  test("Checking GeneratorProfile.powerString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setPowerString("something")
    expect(x.powerString()).toBe("something")
  });
  test("Checking GeneratorProfile.squareRootString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setSquareRootString("something")
    expect(x.squareRootString()).toBe("something")
  });
  test("Checking GeneratorProfile.squareString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setSquareString("something")
    expect(x.squareString()).toBe("something")
  });
  test("Checking GeneratorProfile.absoluteValueString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAbsoluteValueString("something")
    expect(x.absoluteValueString()).toBe("something")
  });
  test("Checking GeneratorProfile.exponentialString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setExponentialString("something")
    expect(x.exponentialString()).toBe("something")
  });
  test("Checking GeneratorProfile.naturalLogarithmString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setNaturalLogarithmString("something")
    expect(x.naturalLogarithmString()).toBe("something")
  });
  test("Checking GeneratorProfile.commonLogarithmString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCommonLogarithmString("something")
    expect(x.commonLogarithmString()).toBe("something")
  });
  test("Checking GeneratorProfile.ceilingString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCeilingString("something")
    expect(x.ceilingString()).toBe("something")
  });
  test("Checking GeneratorProfile.floorString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setFloorString("something")
    expect(x.floorString()).toBe("something")
  });
  test("Checking GeneratorProfile.minString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setMinString("something")
    expect(x.minString()).toBe("something")
  });
  test("Checking GeneratorProfile.maxString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setMaxString("something")
    expect(x.maxString()).toBe("something")
  });
  test("Checking GeneratorProfile.remString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setRemString("something")
    expect(x.remString()).toBe("something")
  });
  test("Checking GeneratorProfile.hasPowerOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasPowerOperator(true)
    expect(x.hasPowerOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.sinString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setSinString("something")
    expect(x.sinString()).toBe("something")
  });
  test("Checking GeneratorProfile.cosString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCosString("something")
    expect(x.cosString()).toBe("something")
  });
  test("Checking GeneratorProfile.tanString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setTanString("something")
    expect(x.tanString()).toBe("something")
  });
  test("Checking GeneratorProfile.secString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setSecString("something")
    expect(x.secString()).toBe("something")
  });
  test("Checking GeneratorProfile.cscString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCscString("something")
    expect(x.cscString()).toBe("something")
  });
  test("Checking GeneratorProfile.cotString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCotString("something")
    expect(x.cotString()).toBe("something")
  });
  test("Checking GeneratorProfile.sinhString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setSinhString("something")
    expect(x.sinhString()).toBe("something")
  });
  test("Checking GeneratorProfile.coshString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCoshString("something")
    expect(x.coshString()).toBe("something")
  });
  test("Checking GeneratorProfile.tanhString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setTanhString("something")
    expect(x.tanhString()).toBe("something")
  });
  test("Checking GeneratorProfile.sechString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setSechString("something")
    expect(x.sechString()).toBe("something")
  });
  test("Checking GeneratorProfile.cschString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCschString("something")
    expect(x.cschString()).toBe("something")
  });
  test("Checking GeneratorProfile.cothString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCothString("something")
    expect(x.cothString()).toBe("something")
  });
  test("Checking GeneratorProfile.asinString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAsinString("something")
    expect(x.asinString()).toBe("something")
  });
  test("Checking GeneratorProfile.acosString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAcosString("something")
    expect(x.acosString()).toBe("something")
  });
  test("Checking GeneratorProfile.atanString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAtanString("something")
    expect(x.atanString()).toBe("something")
  });
  test("Checking GeneratorProfile.asecString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAsecString("something")
    expect(x.asecString()).toBe("something")
  });
  test("Checking GeneratorProfile.acscString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAcscString("something")
    expect(x.acscString()).toBe("something")
  });
  test("Checking GeneratorProfile.acotString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAcotString("something")
    expect(x.acotString()).toBe("something")
  });
  test("Checking GeneratorProfile.asinhString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAsinhString("something")
    expect(x.asinhString()).toBe("something")
  });
  test("Checking GeneratorProfile.acoshString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAcoshString("something")
    expect(x.acoshString()).toBe("something")
  });
  test("Checking GeneratorProfile.atanhString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAtanhString("something")
    expect(x.atanhString()).toBe("something")
  });
  test("Checking GeneratorProfile.asechString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAsechString("something")
    expect(x.asechString()).toBe("something")
  });
  test("Checking GeneratorProfile.acschString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAcschString("something")
    expect(x.acschString()).toBe("something")
  });
  test("Checking GeneratorProfile.acothString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAcothString("something")
    expect(x.acothString()).toBe("something")
  });
  test("Checking GeneratorProfile.conditionalOperatorIfString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setConditionalOperatorIfString("something")
    expect(x.conditionalOperatorIfString()).toBe("something")
  });
  test("Checking GeneratorProfile.conditionalOperatorElseString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setConditionalOperatorElseString("something")
    expect(x.conditionalOperatorElseString()).toBe("something")
  });
  test("Checking GeneratorProfile.piecewiseIfString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setPiecewiseIfString("something")
    expect(x.piecewiseIfString()).toBe("something")
  });
  test("Checking GeneratorProfile.piecewiseElseString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setPiecewiseElseString("something")
    expect(x.piecewiseElseString()).toBe("something")
  });
  test("Checking GeneratorProfile.hasConditionalOperator.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setHasConditionalOperator(true)
    expect(x.hasConditionalOperator()).toBe(true)
  });
  test("Checking GeneratorProfile.trueString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setTrueString("something")
    expect(x.trueString()).toBe("something")
  });
  test("Checking GeneratorProfile.falseString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setFalseString("something")
    expect(x.falseString()).toBe("something")
  });
  test("Checking GeneratorProfile.eString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setEString("something")
    expect(x.eString()).toBe("something")
  });
  test("Checking GeneratorProfile.piString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setPiString("something")
    expect(x.piString()).toBe("something")
  });
  test("Checking GeneratorProfile.infString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInfString("something")
    expect(x.infString()).toBe("something")
  });
  test("Checking GeneratorProfile.nanString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setNanString("something")
    expect(x.nanString()).toBe("something")
  });
  test("Checking GeneratorProfile.eqFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setEqFunctionString("something")
    expect(x.eqFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.neqFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setNeqFunctionString("something")
    expect(x.neqFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.ltFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setLtFunctionString("something")
    expect(x.ltFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.leqFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setLeqFunctionString("something")
    expect(x.leqFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.gtFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setGtFunctionString("something")
    expect(x.gtFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.geqFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setGeqFunctionString("something")
    expect(x.geqFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.andFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAndFunctionString("something")
    expect(x.andFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.orFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setOrFunctionString("something")
    expect(x.orFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.xorFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setXorFunctionString("something")
    expect(x.xorFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.notFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setNotFunctionString("something")
    expect(x.notFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.minFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setMinFunctionString("something")
    expect(x.minFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.maxFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setMaxFunctionString("something")
    expect(x.maxFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.secFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setSecFunctionString("something")
    expect(x.secFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.cscFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCscFunctionString("something")
    expect(x.cscFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.cotFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCotFunctionString("something")
    expect(x.cotFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.sechFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setSechFunctionString("something")
    expect(x.sechFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.cschFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCschFunctionString("something")
    expect(x.cschFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.cothFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCothFunctionString("something")
    expect(x.cothFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.asecFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAsecFunctionString("something")
    expect(x.asecFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.acscFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAcscFunctionString("something")
    expect(x.acscFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.acotFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAcotFunctionString("something")
    expect(x.acotFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.asechFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAsechFunctionString("something")
    expect(x.asechFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.acschFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAcschFunctionString("something")
    expect(x.acschFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.acothFunctionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAcothFunctionString("something")
    expect(x.acothFunctionString()).toBe("something")
  });
  test("Checking GeneratorProfile.commentString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCommentString("something")
    expect(x.commentString()).toBe("something")
  });
  test("Checking GeneratorProfile.originCommentString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setOriginCommentString("something")
    expect(x.originCommentString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceFileNameString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceFileNameString("something")
    expect(x.interfaceFileNameString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceHeaderString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceHeaderString("something")
    expect(x.interfaceHeaderString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationHeaderString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationHeaderString("something")
    expect(x.implementationHeaderString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceVersionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceVersionString("something")
    expect(x.interfaceVersionString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationVersionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationVersionString("something")
    expect(x.implementationVersionString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceLibcellmlVersionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceLibcellmlVersionString("something")
    expect(x.interfaceLibcellmlVersionString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationLibcellmlVersionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationLibcellmlVersionString("something")
    expect(x.implementationLibcellmlVersionString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceStateCountString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceStateCountString("something")
    expect(x.interfaceStateCountString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationStateCountString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationStateCountString("something")
    expect(x.implementationStateCountString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceConstantCountString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceConstantCountString("something")
    expect(x.interfaceConstantCountString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationConstantCountString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationConstantCountString("something")
    expect(x.implementationConstantCountString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceComputedConstantCountString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceComputedConstantCountString("something")
    expect(x.interfaceComputedConstantCountString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationComputedConstantCountString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationComputedConstantCountString("something")
    expect(x.implementationComputedConstantCountString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceAlgebraicVariableCountString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceAlgebraicVariableCountString("something")
    expect(x.interfaceAlgebraicVariableCountString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationAlgebraicVariableCountString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationAlgebraicVariableCountString("something")
    expect(x.implementationAlgebraicVariableCountString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceExternalVariableCountString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceExternalVariableCountString("something")
    expect(x.interfaceExternalVariableCountString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationExternalVariableCountString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationExternalVariableCountString("something")
    expect(x.implementationExternalVariableCountString()).toBe("something")
  });
  test("Checking GeneratorProfile.variableInfoObjectString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setVariableInfoObjectString("something")
    expect(x.variableInfoObjectString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceVoiInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceVoiInfoString("something")
    expect(x.interfaceVoiInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationVoiInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationVoiInfoString("something")
    expect(x.implementationVoiInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceStateInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceStateInfoString("something")
    expect(x.interfaceStateInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationStateInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationStateInfoString("something")
    expect(x.implementationStateInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceConstantInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceConstantInfoString("something")
    expect(x.interfaceConstantInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationConstantInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationConstantInfoString("something")
    expect(x.implementationConstantInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceComputedConstantInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceComputedConstantInfoString("something")
    expect(x.interfaceComputedConstantInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationComputedConstantInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationComputedConstantInfoString("something")
    expect(x.implementationComputedConstantInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceAlgebraicVariableInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceAlgebraicVariableInfoString("something")
    expect(x.interfaceAlgebraicVariableInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationAlgebraicVariableInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationAlgebraicVariableInfoString("something")
    expect(x.implementationAlgebraicVariableInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceExternalVariableInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceExternalVariableInfoString("something")
    expect(x.interfaceExternalVariableInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationExternalVariableInfoString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationExternalVariableInfoString("something")
    expect(x.implementationExternalVariableInfoString()).toBe("something")
  });
  test("Checking GeneratorProfile.variableInfoEntryString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setVariableInfoEntryString("something")
    expect(x.variableInfoEntryString()).toBe("something")
  });
  test("Checking GeneratorProfile.rootFindingInfoObjectString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setRootFindingInfoObjectString(false, false, "something")
    expect(x.rootFindingInfoObjectString(false, false)).toBe("something")

    x.setRootFindingInfoObjectString(false, true, "something")
    expect(x.rootFindingInfoObjectString(false, true)).toBe("something")

    x.setRootFindingInfoObjectString(true, false, "something")
    expect(x.rootFindingInfoObjectString(true, false)).toBe("something")

    x.setRootFindingInfoObjectString(true, true, "something")
    expect(x.rootFindingInfoObjectString(true, true)).toBe("something")
  });
  test("Checking GeneratorProfile.externNlaSolveMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setExternNlaSolveMethodString("something")
    expect(x.externNlaSolveMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.findRootCallString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setFindRootCallString(false, false, "something")
    expect(x.findRootCallString(false, false)).toBe("something")

    x.setFindRootCallString(false, true, "something")
    expect(x.findRootCallString(false, true)).toBe("something")

    x.setFindRootCallString(true, false, "something")
    expect(x.findRootCallString(true, false)).toBe("something")

    x.setFindRootCallString(true, true, "something")
    expect(x.findRootCallString(true, true)).toBe("something")
  });
  test("Checking GeneratorProfile.findRootMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setFindRootMethodString(false, false, "something")
    expect(x.findRootMethodString(false, false)).toBe("something")

    x.setFindRootMethodString(false, true, "something")
    expect(x.findRootMethodString(false, true)).toBe("something")

    x.setFindRootMethodString(true, false, "something")
    expect(x.findRootMethodString(true, false)).toBe("something")

    x.setFindRootMethodString(true, true, "something")
    expect(x.findRootMethodString(true, true)).toBe("something")
  });
  test("Checking GeneratorProfile.nlaSolveCallString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setNlaSolveCallString(false, false, "something")
    expect(x.nlaSolveCallString(false, false)).toBe("something")

    x.setNlaSolveCallString(false, true, "something")
    expect(x.nlaSolveCallString(false, true)).toBe("something")

    x.setNlaSolveCallString(true, false, "something")
    expect(x.nlaSolveCallString(true, false)).toBe("something")

    x.setNlaSolveCallString(true, true, "something")
    expect(x.nlaSolveCallString(true, true)).toBe("something")
  });
  test("Checking GeneratorProfile.objectiveFunctionMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setObjectiveFunctionMethodString(false, false, "something")
    expect(x.objectiveFunctionMethodString(false, false)).toBe("something")

    x.setObjectiveFunctionMethodString(false, true, "something")
    expect(x.objectiveFunctionMethodString(false, true)).toBe("something")

    x.setObjectiveFunctionMethodString(true, false, "something")
    expect(x.objectiveFunctionMethodString(true, false)).toBe("something")

    x.setObjectiveFunctionMethodString(true, true, "something")
    expect(x.objectiveFunctionMethodString(true, true)).toBe("something")
  });
  test("Checking GeneratorProfile.uArrayString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setUArrayString("something")
    expect(x.uArrayString()).toBe("something")
  });
  test("Checking GeneratorProfile.fArrayString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setFArrayString("something")
    expect(x.fArrayString()).toBe("something")
  });
  test("Checking GeneratorProfile.voiString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setVoiString("something")
    expect(x.voiString()).toBe("something")
  });
  test("Checking GeneratorProfile.statesArrayString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setStatesArrayString("something")
    expect(x.statesArrayString()).toBe("something")
  });
  test("Checking GeneratorProfile.ratesArrayString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setRatesArrayString("something")
    expect(x.ratesArrayString()).toBe("something")
  });
  test("Checking GeneratorProfile.constantsArrayString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setConstantsArrayString("something")
    expect(x.constantsArrayString()).toBe("something")
  });
  test("Checking GeneratorProfile.computedConstantsArrayString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setComputedConstantsArrayString("something")
    expect(x.computedConstantsArrayString()).toBe("something")
  });
  test("Checking GeneratorProfile.algebraicVariablesArrayString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setAlgebraicVariablesArrayString("something")
    expect(x.algebraicVariablesArrayString()).toBe("something")
  });
  test("Checking GeneratorProfile.externalVariablesArrayString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setExternalVariablesArrayString("something")
    expect(x.externalVariablesArrayString()).toBe("something")
  });
  test("Checking GeneratorProfile.externalVariableMethodTypeDefinitionString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setExternalVariableMethodTypeDefinitionString(false, "something")
    expect(x.externalVariableMethodTypeDefinitionString(false)).toBe("something")

    x.setExternalVariableMethodTypeDefinitionString(true, "something")
    expect(x.externalVariableMethodTypeDefinitionString(true)).toBe("something")
  });
  test("Checking GeneratorProfile.externalVariableMethodCallString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setExternalVariableMethodCallString(false, "something")
    expect(x.externalVariableMethodCallString(false)).toBe("something")

    x.setExternalVariableMethodCallString(true, "something")
    expect(x.externalVariableMethodCallString(true)).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceCreateStatesArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceCreateStatesArrayMethodString("something")
    expect(x.interfaceCreateStatesArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationCreateStatesArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationCreateStatesArrayMethodString("something")
    expect(x.implementationCreateStatesArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceCreateConstantsArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceCreateConstantsArrayMethodString("something")
    expect(x.interfaceCreateConstantsArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationCreateConstantsArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationCreateConstantsArrayMethodString("something")
    expect(x.implementationCreateConstantsArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceCreateComputedConstantsArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceCreateComputedConstantsArrayMethodString("something")
    expect(x.interfaceCreateComputedConstantsArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationCreateComputedConstantsArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationCreateComputedConstantsArrayMethodString("something")
    expect(x.implementationCreateComputedConstantsArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceCreateAlgebraicVariablesArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceCreateAlgebraicVariablesArrayMethodString("something")
    expect(x.interfaceCreateAlgebraicVariablesArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationCreateAlgebraicVariablesArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationCreateAlgebraicVariablesArrayMethodString("something")
    expect(x.implementationCreateAlgebraicVariablesArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceCreateExternalVariablesArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceCreateExternalVariablesArrayMethodString("something")
    expect(x.interfaceCreateExternalVariablesArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationCreateExternalVariablesArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationCreateExternalVariablesArrayMethodString("something")
    expect(x.implementationCreateExternalVariablesArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceDeleteArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceDeleteArrayMethodString("something")
    expect(x.interfaceDeleteArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.implementationDeleteArrayMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationDeleteArrayMethodString("something")
    expect(x.implementationDeleteArrayMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceInitialiseArraysMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceInitialiseArraysMethodString(false, "something")
    expect(x.interfaceInitialiseArraysMethodString(false)).toBe("something")

    x.setInterfaceInitialiseArraysMethodString(true, "something")
    expect(x.interfaceInitialiseArraysMethodString(true)).toBe("something")
  });
  test("Checking GeneratorProfile.implementationInitialiseArraysMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationInitialiseArraysMethodString(false, "something")
    expect(x.implementationInitialiseArraysMethodString(false)).toBe("something")

    x.setImplementationInitialiseArraysMethodString(true, "something")
    expect(x.implementationInitialiseArraysMethodString(true)).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceComputeComputedConstantsMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceComputeComputedConstantsMethodString(false, "something")
    expect(x.interfaceComputeComputedConstantsMethodString(false)).toBe("something")

    x.setInterfaceComputeComputedConstantsMethodString(true, "something")
    expect(x.interfaceComputeComputedConstantsMethodString(true)).toBe("something")
  });
  test("Checking GeneratorProfile.implementationComputeComputedConstantsMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationComputeComputedConstantsMethodString(false, "something")
    expect(x.implementationComputeComputedConstantsMethodString(false)).toBe("something")

    x.setImplementationComputeComputedConstantsMethodString(true, "something")
    expect(x.implementationComputeComputedConstantsMethodString(true)).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceComputeRatesMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceComputeRatesMethodString(false, "something")
    expect(x.interfaceComputeRatesMethodString(false)).toBe("something")

    x.setInterfaceComputeRatesMethodString(true, "something")
    expect(x.interfaceComputeRatesMethodString(true)).toBe("something")
  });
  test("Checking GeneratorProfile.implementationComputeRatesMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationComputeRatesMethodString(false, "something")
    expect(x.implementationComputeRatesMethodString(false)).toBe("something")

    x.setImplementationComputeRatesMethodString(true, "something")
    expect(x.implementationComputeRatesMethodString(true)).toBe("something")
  });
  test("Checking GeneratorProfile.interfaceComputeVariablesMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setInterfaceComputeVariablesMethodString(false, false, "something")
    expect(x.interfaceComputeVariablesMethodString(false, false)).toBe("something")

    x.setInterfaceComputeVariablesMethodString(false, true, "something")
    expect(x.interfaceComputeVariablesMethodString(false, true)).toBe("something")

    x.setInterfaceComputeVariablesMethodString(true, false, "something")
    expect(x.interfaceComputeVariablesMethodString(true, false)).toBe("something")

    x.setInterfaceComputeVariablesMethodString(true, true, "something")
    expect(x.interfaceComputeVariablesMethodString(true, true)).toBe("something")
  });
  test("Checking GeneratorProfile.implementationComputeVariablesMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setImplementationComputeVariablesMethodString(false, false, "something")
    expect(x.implementationComputeVariablesMethodString(false, false)).toBe("something")

    x.setImplementationComputeVariablesMethodString(false, true, "something")
    expect(x.implementationComputeVariablesMethodString(false, true)).toBe("something")

    x.setImplementationComputeVariablesMethodString(true, false, "something")
    expect(x.implementationComputeVariablesMethodString(true, false)).toBe("something")

    x.setImplementationComputeVariablesMethodString(true, true, "something")
    expect(x.implementationComputeVariablesMethodString(true, true)).toBe("something")
  });
  test("Checking GeneratorProfile.emptyMethodString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setEmptyMethodString("something")
    expect(x.emptyMethodString()).toBe("something")
  });
  test("Checking GeneratorProfile.indentString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setIndentString("something")
    expect(x.indentString()).toBe("something")
  });
  test("Checking GeneratorProfile.variableDeclarationString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setVariableDeclarationString("something")
    expect(x.variableDeclarationString()).toBe("something")
  });
  test("Checking GeneratorProfile.openArrayString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setOpenArrayString("something")
    expect(x.openArrayString()).toBe("something")
  });
  test("Checking GeneratorProfile.closeArrayString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCloseArrayString("something")
    expect(x.closeArrayString()).toBe("something")
  });
  test("Checking GeneratorProfile.arrayElementSeparatorString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setArrayElementSeparatorString("something")
    expect(x.arrayElementSeparatorString()).toBe("something")
  });
  test("Checking GeneratorProfile.commandSeparatorString.", () => {
    const x = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

    x.setCommandSeparatorString("something")
    expect(x.commandSeparatorString()).toBe("something")
  });
})
