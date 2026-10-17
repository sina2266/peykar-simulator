#
# Tests the GeneratorProfile class bindings
#
import unittest


class GeneratorProfileTestCase(unittest.TestCase):
    VALUE = 'value'

    def test_create_destroy(self):
        from libcellml import GeneratorProfile

        x = GeneratorProfile()
        del x

    def test_generator_profile(self):
        from libcellml import GeneratorProfile
        from libcellml.generatorprofile import GeneratorProfile_profileAsString

        # Create a default, i.e. C, profile.
        p = GeneratorProfile()
        self.assertEqual(GeneratorProfile.Profile.C, p.profile())
        self.assertEqual("c", GeneratorProfile.profileAsString(p.profile()))
        self.assertEqual("c", GeneratorProfile_profileAsString(p.profile()))

        # Make the profile a Python profile.
        p.setProfile(GeneratorProfile.Profile.PYTHON)
        self.assertEqual(GeneratorProfile.Profile.PYTHON, p.profile())
        self.assertEqual("python", GeneratorProfile.profileAsString(p.profile()))
        self.assertEqual("python", GeneratorProfile_profileAsString(p.profile()))

        # Create a Python profile.
        pp = GeneratorProfile(GeneratorProfile.Profile.PYTHON)
        self.assertEqual(GeneratorProfile.Profile.PYTHON, pp.profile())

    @unittest.skip('Create tests script')
    def test_create_tests(self):
        import re
        from libcellml import GeneratorProfile

        p = GeneratorProfile()

        seen = {}

        def working_name(entry):
            name = re.sub('([A-Z]{1})', r'_\1', entry).lower()

            if name.startswith('set'):
                name = name[4:]

            return name

        def ignore(entry):
            if entry.startswith('_') or entry in ['Profile', 'profile', 'this', 'thisown']:
                return True

            return False

        def get_set_names(entry):
            if entry.startswith('set'):
                getter = entry[3:]
                getter = getter[0].lower() + getter[1:]

                return [getter, entry]
            else:
                setter = entry
                setter = 'set' + setter[0].upper() + setter[1:]

                return [entry, setter]

        def print_string_method(entry):
            [getter, setter] = get_set_names(entry)
            content = getattr(p, getter)()
            content = content.replace('\n', '\\n')

            print('        from libcellml import GeneratorProfile')
            print('        ')
            print('        g = GeneratorProfile()')
            print('        ')
            print('        self.assertEqual("{content}", g.{getter}())')
            print('        g.{setter}("Some text content.")')
            print('        self.assertEqual("Some text content.", g.{getter}())')
            print('        ')

        def print_has_method(entry):
            print('        from libcellml import GeneratorProfile')
            print('        ')
            print('        g = GeneratorProfile()')
            print('        ')
            print('        self.assertFalse(g.{0}())'.format(entry))
            print('        ')

        def is_string_method(name):
            return name.endswith('string')

        def is_has_method(name):
            return name.startswith('has')

        for entry in dir(p):
            name = working_name(entry)

            if not ignore(entry) and not name in seen:
                seen[name] = True

                print('    def test_{0}(self):'.format(name))

                if is_string_method(name):
                    print_string_method(entry)
                elif is_has_method(name):
                    print_has_method(entry)
                else:
                    print('        # Yikes missing this method: ' + entry)

    def test_absolute_value_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('fabs', g.absoluteValueString())
        g.setAbsoluteValueString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.absoluteValueString())

    def test_acos_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('acos', g.acosString())
        g.setAcosString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.acosString())

    def test_acosh_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('acosh', g.acoshString())
        g.setAcoshString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.acoshString())

    def test_acot_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double acot(double x)\n{\n    return atan(1.0/x);\n}\n', g.acotFunctionString())
        g.setAcotFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.acotFunctionString())

    def test_acot_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('acot', g.acotString())
        g.setAcotString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.acotString())

    def test_acoth_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'double acoth(double x)\n{\n    double oneOverX = 1.0/x;\n\n    return 0.5*log((1.0+oneOverX)/(1.0-oneOverX));\n}\n',
            g.acothFunctionString())
        g.setAcothFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.acothFunctionString())

    def test_acoth_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('acoth', g.acothString())
        g.setAcothString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.acothString())

    def test_acsc_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double acsc(double x)\n{\n    return asin(1.0/x);\n}\n', g.acscFunctionString())
        g.setAcscFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.acscFunctionString())

    def test_acsc_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('acsc', g.acscString())
        g.setAcscString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.acscString())

    def test_acsch_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'double acsch(double x)\n{\n    double oneOverX = 1.0/x;\n\n    return log(oneOverX+sqrt(oneOverX*oneOverX+1.0));\n}\n',
            g.acschFunctionString())
        g.setAcschFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.acschFunctionString())

    def test_acsch_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('acsch', g.acschString())
        g.setAcschString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.acschString())

    def test_and_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.andFunctionString())
        g.setAndFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.andFunctionString())

    def test_and_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(' && ', g.andString())
        g.setAndString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.andString())

    def test_array_element_separator_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(',', g.arrayElementSeparatorString())
        g.setArrayElementSeparatorString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.arrayElementSeparatorString())

    def test_asec_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double asec(double x)\n{\n    return acos(1.0/x);\n}\n', g.asecFunctionString())
        g.setAsecFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.asecFunctionString())

    def test_asec_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('asec', g.asecString())
        g.setAsecString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.asecString())

    def test_asech_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'double asech(double x)\n{\n    double oneOverX = 1.0/x;\n\n    return log(oneOverX+sqrt(oneOverX*oneOverX-1.0));\n}\n',
            g.asechFunctionString())
        g.setAsechFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.asechFunctionString())

    def test_asech_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('asech', g.asechString())
        g.setAsechString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.asechString())

    def test_asin_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('asin', g.asinString())
        g.setAsinString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.asinString())

    def test_asinh_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('asinh', g.asinhString())
        g.setAsinhString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.asinhString())

    def test_equality_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(' = ', g.equalityString())
        g.setEqualityString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.equalityString())

    def test_atan_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('atan', g.atanString())
        g.setAtanString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.atanString())

    def test_atanh_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('atanh', g.atanhString())
        g.setAtanhString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.atanhString())

    def test_ceiling_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('ceil', g.ceilingString())
        g.setCeilingString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.ceilingString())

    def test_close_array_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(']', g.closeArrayString())
        g.setCloseArrayString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.closeArrayString())

    def test_command_separator_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(';', g.commandSeparatorString())
        g.setCommandSeparatorString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.commandSeparatorString())

    def test_comment_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('/* [CODE] */\n', g.commentString())
        g.setCommentString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.commentString())

    def test_common_logarithm_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('log10', g.commonLogarithmString())
        g.setCommonLogarithmString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.commonLogarithmString())

    def test_conditional_operator_else_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(':[ELSE_STATEMENT]', g.conditionalOperatorElseString())
        g.setConditionalOperatorElseString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.conditionalOperatorElseString())

    def test_conditional_operator_if_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('([CONDITION])?[IF_STATEMENT]', g.conditionalOperatorIfString())
        g.setConditionalOperatorIfString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.conditionalOperatorIfString())

    def test_cos_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('cos', g.cosString())
        g.setCosString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.cosString())

    def test_cosh_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('cosh', g.coshString())
        g.setCoshString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.coshString())

    def test_cot_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double cot(double x)\n{\n    return 1.0/tan(x);\n}\n', g.cotFunctionString())
        g.setCotFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.cotFunctionString())

    def test_cot_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('cot', g.cotString())
        g.setCotString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.cotString())

    def test_coth_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double coth(double x)\n{\n    return 1.0/tanh(x);\n}\n', g.cothFunctionString())
        g.setCothFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.cothFunctionString())

    def test_coth_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('coth', g.cothString())
        g.setCothString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.cothString())

    def test_csc_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double csc(double x)\n{\n    return 1.0/sin(x);\n}\n', g.cscFunctionString())
        g.setCscFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.cscFunctionString())

    def test_csc_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('csc', g.cscString())
        g.setCscString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.cscString())

    def test_csch_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double csch(double x)\n{\n    return 1.0/sinh(x);\n}\n', g.cschFunctionString())
        g.setCschFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.cschFunctionString())

    def test_csch_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('csch', g.cschString())
        g.setCschString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.cschString())

    def test_divide_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('/', g.divideString())
        g.setDivideString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.divideString())

    def test_e_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('2.71828182845905', g.eString())
        g.setEString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.eString())

    def test_empty_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.emptyMethodString())
        g.setEmptyMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.emptyMethodString())

    def test_eq_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.eqFunctionString())
        g.setEqFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.eqFunctionString())

    def test_eq_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(' == ', g.eqString())
        g.setEqString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.eqString())

    def test_exponential_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('exp', g.exponentialString())
        g.setExponentialString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.exponentialString())

    def test_false_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('0.0', g.falseString())
        g.setFalseString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.falseString())

    def test_floor_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('floor', g.floorString())
        g.setFloorString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.floorString())

    def test_geq_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.geqFunctionString())
        g.setGeqFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.geqFunctionString())

    def test_geq_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(' >= ', g.geqString())
        g.setGeqString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.geqString())

    def test_gt_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.gtFunctionString())
        g.setGtFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.gtFunctionString())

    def test_gt_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(' > ', g.gtString())
        g.setGtString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.gtString())

    def test_has_and_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasAndOperator())
        g.setHasAndOperator(False)
        self.assertFalse(g.hasAndOperator())

    def test_has_conditional_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasConditionalOperator())
        g.setHasConditionalOperator(False)
        self.assertFalse(g.hasConditionalOperator())

    def test_has_eq_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasEqOperator())
        g.setHasEqOperator(False)
        self.assertFalse(g.hasEqOperator())

    def test_has_geq_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasGeqOperator())
        g.setHasGeqOperator(False)
        self.assertFalse(g.hasGeqOperator())

    def test_has_gt_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasGtOperator())
        g.setHasGtOperator(False)
        self.assertFalse(g.hasGtOperator())

    def test_has_leq_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasLeqOperator())
        g.setHasLeqOperator(False)
        self.assertFalse(g.hasLeqOperator())

    def test_has_lt_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasLtOperator())
        g.setHasLtOperator(False)
        self.assertFalse(g.hasLtOperator())

    def test_has_neq_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasNeqOperator())
        g.setHasNeqOperator(False)
        self.assertFalse(g.hasNeqOperator())

    def test_has_not_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasNotOperator())
        g.setHasNotOperator(False)
        self.assertFalse(g.hasNotOperator())

    def test_has_or_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasOrOperator())
        g.setHasOrOperator(False)
        self.assertFalse(g.hasOrOperator())

    def test_has_power_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertFalse(g.hasPowerOperator())
        g.setHasPowerOperator(True)
        self.assertTrue(g.hasPowerOperator())

    def test_has_xor_operator(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertFalse(g.hasXorOperator())
        g.setHasXorOperator(True)
        self.assertTrue(g.hasXorOperator())

    def test_implementation_compute_computed_constants_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('void computeComputedConstants(double *constants, double *computedConstants, double *algebraicVariables)\n{\n[CODE]}\n',
                         g.implementationComputeComputedConstantsMethodString(False))
        g.setImplementationComputeComputedConstantsMethodString(False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationComputeComputedConstantsMethodString(False))

        self.assertEqual('void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)\n{\n[CODE]}\n',
                         g.implementationComputeComputedConstantsMethodString(True))
        g.setImplementationComputeComputedConstantsMethodString(True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationComputeComputedConstantsMethodString(True))

    def test_implementation_compute_rates_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)\n{\n[CODE]}\n',
            g.implementationComputeRatesMethodString(False))
        g.setImplementationComputeRatesMethodString(False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationComputeRatesMethodString(False))

        self.assertEqual(
            'void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)\n{\n[CODE]}\n',
            g.implementationComputeRatesMethodString(True))
        g.setImplementationComputeRatesMethodString(True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationComputeRatesMethodString(True))

    def test_implementation_compute_variables_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'void computeVariables(double *constants, double *computedConstants, double *algebraicVariables)\n{\n[CODE]}\n',
            g.implementationComputeVariablesMethodString(False, False))
        g.setImplementationComputeVariablesMethodString(False, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationComputeVariablesMethodString(False, False))

        self.assertEqual(
            'void computeVariables(double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)\n{\n[CODE]}\n',
            g.implementationComputeVariablesMethodString(False, True))
        g.setImplementationComputeVariablesMethodString(False, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationComputeVariablesMethodString(False, True))

        self.assertEqual(
            'void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)\n{\n[CODE]}\n',
            g.implementationComputeVariablesMethodString(True, False))
        g.setImplementationComputeVariablesMethodString(True, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationComputeVariablesMethodString(True, False))

        self.assertEqual(
            'void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)\n{\n[CODE]}\n',
            g.implementationComputeVariablesMethodString(True, True))
        g.setImplementationComputeVariablesMethodString(True, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationComputeVariablesMethodString(True, True))

    def test_implementation_create_states_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'double * createStatesArray()\n{\n    double *res = (double *) malloc(STATE_COUNT*sizeof(double));\n\n    for (size_t i = 0; i < STATE_COUNT; ++i) {\n        res[i] = NAN;\n    }\n\n    return res;\n}\n',
            g.implementationCreateStatesArrayMethodString())
        g.setImplementationCreateStatesArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationCreateStatesArrayMethodString())

    def test_implementation_create_constants_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'double * createConstantsArray()\n{\n    double *res = (double *) malloc(CONSTANT_COUNT*sizeof(double));\n\n    for (size_t i = 0; i < CONSTANT_COUNT; ++i) {\n        res[i] = NAN;\n    }\n\n    return res;\n}\n',
            g.implementationCreateConstantsArrayMethodString())
        g.setImplementationCreateConstantsArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationCreateConstantsArrayMethodString())

    def test_implementation_create_computed_constants_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'double * createComputedConstantsArray()\n{\n    double *res = (double *) malloc(COMPUTED_CONSTANT_COUNT*sizeof(double));\n\n    for (size_t i = 0; i < COMPUTED_CONSTANT_COUNT; ++i) {\n        res[i] = NAN;\n    }\n\n    return res;\n}\n',
            g.implementationCreateComputedConstantsArrayMethodString())
        g.setImplementationCreateComputedConstantsArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationCreateComputedConstantsArrayMethodString())

    def test_implementation_create_algebraic_variables_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'double * createAlgebraicVariablesArray()\n{\n    double *res = (double *) malloc(ALGEBRAIC_VARIABLE_COUNT*sizeof(double));\n\n    for (size_t i = 0; i < ALGEBRAIC_VARIABLE_COUNT; ++i) {\n        res[i] = NAN;\n    }\n\n    return res;\n}\n',
            g.implementationCreateAlgebraicVariablesArrayMethodString())
        g.setImplementationCreateAlgebraicVariablesArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationCreateAlgebraicVariablesArrayMethodString())

    def test_implementation_create_externals_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'double * createExternalVariablesArray()\n{\n    double *res = (double *) malloc(EXTERNAL_VARIABLE_COUNT*sizeof(double));\n\n    for (size_t i = 0; i < EXTERNAL_VARIABLE_COUNT; ++i) {\n        res[i] = NAN;\n    }\n\n    return res;\n}\n',
            g.implementationCreateExternalVariablesArrayMethodString())
        g.setImplementationCreateExternalVariablesArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationCreateExternalVariablesArrayMethodString())

    def test_implementation_delete_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('void deleteArray(double *array)\n{\n    free(array);\n}\n',
                         g.implementationDeleteArrayMethodString())
        g.setImplementationDeleteArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationDeleteArrayMethodString())

    def test_implementation_header_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('#include "[INTERFACE_FILE_NAME]"\n\n#include <math.h>\n#include <stdlib.h>\n',
                         g.implementationHeaderString())
        g.setImplementationHeaderString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationHeaderString())

    def test_implementation_initialise_constants_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('void initialiseArrays(double *constants, double *computedConstants, double *algebraicVariables)\n{\n[CODE]}\n',
                         g.implementationInitialiseArraysMethodString(False))
        g.setImplementationInitialiseArraysMethodString(False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationInitialiseArraysMethodString(False))

        self.assertEqual('void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)\n{\n[CODE]}\n',
                         g.implementationInitialiseArraysMethodString(True))
        g.setImplementationInitialiseArraysMethodString(True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationInitialiseArraysMethodString(True))

    def test_implementation_libcellml_version_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const char LIBCELLML_VERSION[] = "[LIBCELLML_VERSION]";\n',
                         g.implementationLibcellmlVersionString())
        g.setImplementationLibcellmlVersionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationLibcellmlVersionString())

    def test_implementation_state_count_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const size_t STATE_COUNT = [STATE_COUNT];\n', g.implementationStateCountString())
        g.setImplementationStateCountString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationStateCountString())

    def test_implementation_state_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const VariableInfo STATE_INFO[] = {\n[CODE]};\n', g.implementationStateInfoString())
        g.setImplementationStateInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationStateInfoString())

    def test_implementation_constant_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const VariableInfo CONSTANT_INFO[] = {\n[CODE]};\n', g.implementationConstantInfoString())
        g.setImplementationConstantInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationConstantInfoString())

    def test_implementation_computed_constant_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const VariableInfo COMPUTED_CONSTANT_INFO[] = {\n[CODE]};\n', g.implementationComputedConstantInfoString())
        g.setImplementationComputedConstantInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationComputedConstantInfoString())

    def test_implementation_algebraic_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {\n[CODE]};\n', g.implementationAlgebraicVariableInfoString())
        g.setImplementationAlgebraicVariableInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationAlgebraicVariableInfoString())

    def test_implementation_external_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const VariableInfo EXTERNAL_VARIABLE_INFO[] = {\n[CODE]};\n', g.implementationExternalVariableInfoString())
        g.setImplementationExternalVariableInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationExternalVariableInfoString())

    def test_implementation_constant_count_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const size_t CONSTANT_COUNT = [CONSTANT_COUNT];\n', g.implementationConstantCountString())
        g.setImplementationConstantCountString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationConstantCountString())

    def test_implementation_computed_constant_count_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const size_t COMPUTED_CONSTANT_COUNT = [COMPUTED_CONSTANT_COUNT];\n', g.implementationComputedConstantCountString())
        g.setImplementationComputedConstantCountString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationComputedConstantCountString())

    def test_implementation_algebraic_variable_count_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const size_t ALGEBRAIC_VARIABLE_COUNT = [ALGEBRAIC_VARIABLE_COUNT];\n', g.implementationAlgebraicVariableCountString())
        g.setImplementationAlgebraicVariableCountString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationAlgebraicVariableCountString())

    def test_implementation_external_variable_count_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const size_t EXTERNAL_VARIABLE_COUNT = [EXTERNAL_VARIABLE_COUNT];\n', g.implementationExternalVariableCountString())
        g.setImplementationExternalVariableCountString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationExternalVariableCountString())

    def test_implementation_version_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const char VERSION[] = "0.8.0";\n', g.implementationVersionString())
        g.setImplementationVersionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationVersionString())

    def test_implementation_voi_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('const VariableInfo VOI_INFO = [CODE];\n', g.implementationVoiInfoString())
        g.setImplementationVoiInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.implementationVoiInfoString())

    def test_indent_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('    ', g.indentString())
        g.setIndentString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.indentString())

    def test_inf_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('INFINITY', g.infString())
        g.setInfString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.infString())

    def test_interface_compute_computed_constants_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('void computeComputedConstants(double *constants, double *computedConstants, double *algebraicVariables);\n',
                         g.interfaceComputeComputedConstantsMethodString(False))
        g.setInterfaceComputeComputedConstantsMethodString(False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceComputeComputedConstantsMethodString(False))

        self.assertEqual('void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);\n',
                         g.interfaceComputeComputedConstantsMethodString(True))
        g.setInterfaceComputeComputedConstantsMethodString(True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceComputeComputedConstantsMethodString(True))

    def test_interface_compute_rates_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);\n',
            g.interfaceComputeRatesMethodString(False))
        g.setInterfaceComputeRatesMethodString(False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceComputeRatesMethodString(False))

        self.assertEqual(
            'void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable);\n',
            g.interfaceComputeRatesMethodString(True))
        g.setInterfaceComputeRatesMethodString(True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceComputeRatesMethodString(True))

    def test_interface_compute_variables_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'void computeVariables(double *constants, double *computedConstants, double *algebraicVariables);\n',
            g.interfaceComputeVariablesMethodString(False, False))
        g.setInterfaceComputeVariablesMethodString(False, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceComputeVariablesMethodString(False, False))

        self.assertEqual(
            'void computeVariables(double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable);\n',
            g.interfaceComputeVariablesMethodString(False, True))
        g.setInterfaceComputeVariablesMethodString(False, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceComputeVariablesMethodString(False, True))

        self.assertEqual(
            'void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);\n',
            g.interfaceComputeVariablesMethodString(True, False))
        g.setInterfaceComputeVariablesMethodString(True, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceComputeVariablesMethodString(True, False))

        self.assertEqual(
            'void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable);\n',
            g.interfaceComputeVariablesMethodString(True, True))
        g.setInterfaceComputeVariablesMethodString(True, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceComputeVariablesMethodString(True, True))

    def test_interface_create_states_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double * createStatesArray();\n', g.interfaceCreateStatesArrayMethodString())
        g.setInterfaceCreateStatesArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceCreateStatesArrayMethodString())

    def test_interface_create_constants_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double * createConstantsArray();\n', g.interfaceCreateConstantsArrayMethodString())
        g.setInterfaceCreateConstantsArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceCreateConstantsArrayMethodString())

    def test_interface_create_computed_constants_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double * createComputedConstantsArray();\n', g.interfaceCreateComputedConstantsArrayMethodString())
        g.setInterfaceCreateComputedConstantsArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceCreateComputedConstantsArrayMethodString())

    def test_interface_create_algebraic_variables_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double * createAlgebraicVariablesArray();\n', g.interfaceCreateAlgebraicVariablesArrayMethodString())
        g.setInterfaceCreateAlgebraicVariablesArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceCreateAlgebraicVariablesArrayMethodString())

    def test_interface_create_externals_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double * createExternalVariablesArray();\n', g.interfaceCreateExternalVariablesArrayMethodString())
        g.setInterfaceCreateExternalVariablesArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceCreateExternalVariablesArrayMethodString())

    def test_interface_delete_array_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('void deleteArray(double *array);\n', g.interfaceDeleteArrayMethodString())
        g.setInterfaceDeleteArrayMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceDeleteArrayMethodString())

    def test_interface_file_name_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('model.h', g.interfaceFileNameString())
        g.setInterfaceFileNameString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceFileNameString())

    def test_interface_header_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('#pragma once\n\n#include <stddef.h>\n', g.interfaceHeaderString())
        g.setInterfaceHeaderString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceHeaderString())

    def test_interface_initialise_constants_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('void initialiseArrays(double *constants, double *computedConstants, double *algebraicVariables);\n',
                         g.interfaceInitialiseArraysMethodString(False))
        g.setInterfaceInitialiseArraysMethodString(False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceInitialiseArraysMethodString(False))

        self.assertEqual('void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);\n',
                         g.interfaceInitialiseArraysMethodString(True))
        g.setInterfaceInitialiseArraysMethodString(True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceInitialiseArraysMethodString(True))

    def test_interface_libcellml_version_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const char LIBCELLML_VERSION[];\n', g.interfaceLibcellmlVersionString())
        g.setInterfaceLibcellmlVersionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceLibcellmlVersionString())

    def test_interface_state_count_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const size_t STATE_COUNT;\n', g.interfaceStateCountString())
        g.setInterfaceStateCountString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceStateCountString())

    def test_interface_state_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const VariableInfo STATE_INFO[];\n', g.interfaceStateInfoString())
        g.setInterfaceStateInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceStateInfoString())

    def test_interface_constant_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const VariableInfo CONSTANT_INFO[];\n', g.interfaceConstantInfoString())
        g.setInterfaceConstantInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceConstantInfoString())

    def test_interface_computed_constant_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const VariableInfo COMPUTED_CONSTANT_INFO[];\n', g.interfaceComputedConstantInfoString())
        g.setInterfaceComputedConstantInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceComputedConstantInfoString())

    def test_interface_algebraic_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const VariableInfo ALGEBRAIC_VARIABLE_INFO[];\n', g.interfaceAlgebraicVariableInfoString())
        g.setInterfaceAlgebraicVariableInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceAlgebraicVariableInfoString())

    def test_interface_external_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const VariableInfo EXTERNAL_VARIABLE_INFO[];\n', g.interfaceExternalVariableInfoString())
        g.setInterfaceExternalVariableInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceExternalVariableInfoString())

    def test_interface_constant_count_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const size_t CONSTANT_COUNT;\n', g.interfaceConstantCountString())
        g.setInterfaceConstantCountString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceConstantCountString())

    def test_interface_computed_constant_count_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const size_t COMPUTED_CONSTANT_COUNT;\n', g.interfaceComputedConstantCountString())
        g.setInterfaceComputedConstantCountString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceComputedConstantCountString())

    def test_interface_algebraic_variable_count_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const size_t ALGEBRAIC_VARIABLE_COUNT;\n', g.interfaceAlgebraicVariableCountString())
        g.setInterfaceAlgebraicVariableCountString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceAlgebraicVariableCountString())

    def test_interface_external_variable_count_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const size_t EXTERNAL_VARIABLE_COUNT;\n', g.interfaceExternalVariableCountString())
        g.setInterfaceExternalVariableCountString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceExternalVariableCountString())

    def test_interface_version_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const char VERSION[];\n', g.interfaceVersionString())
        g.setInterfaceVersionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceVersionString())

    def test_interface_voi_info_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern const VariableInfo VOI_INFO;\n', g.interfaceVoiInfoString())
        g.setInterfaceVoiInfoString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.interfaceVoiInfoString())

    def test_leq_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.leqFunctionString())
        g.setLeqFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.leqFunctionString())

    def test_leq_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(' <= ', g.leqString())
        g.setLeqString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.leqString())

    def test_lt_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.ltFunctionString())
        g.setLtFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.ltFunctionString())

    def test_lt_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(' < ', g.ltString())
        g.setLtString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.ltString())

    def test_max_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double max(double x, double y)\n{\n    return (x > y)?x:y;\n}\n', g.maxFunctionString())
        g.setMaxFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.maxFunctionString())

    def test_max_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('max', g.maxString())
        g.setMaxString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.maxString())

    def test_min_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double min(double x, double y)\n{\n    return (x < y)?x:y;\n}\n', g.minFunctionString())
        g.setMinFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.minFunctionString())

    def test_min_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('min', g.minString())
        g.setMinString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.minString())

    def test_minus_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('-', g.minusString())
        g.setMinusString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.minusString())

    def test_nan_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('NAN', g.nanString())
        g.setNanString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.nanString())

    def test_natural_logarithm_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('log', g.naturalLogarithmString())
        g.setNaturalLogarithmString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.naturalLogarithmString())

    def test_neq_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.neqFunctionString())
        g.setNeqFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.neqFunctionString())

    def test_neq_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(' != ', g.neqString())
        g.setNeqString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.neqString())

    def test_not_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.notFunctionString())
        g.setNotFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.notFunctionString())

    def test_not_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('!', g.notString())
        g.setNotString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.notString())

    def test_double_declaration_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double [CODE]', g.variableDeclarationString())
        g.setVariableDeclarationString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.variableDeclarationString())

    def test_open_array_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('[', g.openArrayString())
        g.setOpenArrayString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.openArrayString())

    def test_or_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.orFunctionString())
        g.setOrFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.orFunctionString())

    def test_or_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(' || ', g.orString())
        g.setOrString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.orString())

    def test_origin_comment_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'The content of this file was generated using [PROFILE_INFORMATION] libCellML [LIBCELLML_VERSION].',
            g.originCommentString())
        g.setOriginCommentString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.originCommentString())

    def test_pi_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('3.14159265358979', g.piString())
        g.setPiString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.piString())

    def test_piecewise_else_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.piecewiseElseString())
        g.setPiecewiseElseString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.piecewiseElseString())

    def test_piecewise_if_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.piecewiseIfString())
        g.setPiecewiseIfString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.piecewiseIfString())

    def test_plus_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('+', g.plusString())
        g.setPlusString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.plusString())

    def test_power_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('pow', g.powerString())
        g.setPowerString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.powerString())

    def test_rates_array_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('rates', g.ratesArrayString())
        g.setRatesArrayString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.ratesArrayString())

    def test_rem_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('fmod', g.remString())
        g.setRemString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.remString())

    def test_sec_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double sec(double x)\n{\n    return 1.0/cos(x);\n}\n', g.secFunctionString())
        g.setSecFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.secFunctionString())

    def test_sec_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('sec', g.secString())
        g.setSecString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.secString())

    def test_sech_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double sech(double x)\n{\n    return 1.0/cosh(x);\n}\n', g.sechFunctionString())
        g.setSechFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.sechFunctionString())

    def test_sech_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('sech', g.sechString())
        g.setSechString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.sechString())

    def test_sin_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('sin', g.sinString())
        g.setSinString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.sinString())

    def test_sinh_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('sinh', g.sinhString())
        g.setSinhString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.sinhString())

    def test_square_root_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('sqrt', g.squareRootString())
        g.setSquareRootString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.squareRootString())

    def test_square_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('', g.squareString())
        g.setSquareString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.squareString())

    def test_states_array_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('states', g.statesArrayString())
        g.setStatesArrayString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.statesArrayString())

    def test_tan_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('tan', g.tanString())
        g.setTanString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.tanString())

    def test_tanh_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('tanh', g.tanhString())
        g.setTanhString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.tanhString())

    def test_times_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('*', g.timesString())
        g.setTimesString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.timesString())

    def test_true_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('1.0', g.trueString())
        g.setTrueString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.trueString())

    def test_variable_info_entry_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('{"[NAME]", "[UNITS]", "[COMPONENT]"}', g.variableInfoEntryString())
        g.setVariableInfoEntryString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.variableInfoEntryString())

    def test_variable_info_object_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'typedef struct {\n    char name[[NAME_SIZE]];\n    char units[[UNITS_SIZE]];\n    char component[[COMPONENT_SIZE]];\n} VariableInfo;\n',
            g.variableInfoObjectString())
        g.setVariableInfoObjectString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.variableInfoObjectString())

    def test_constants_array_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('constants', g.constantsArrayString())
        g.setConstantsArrayString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.constantsArrayString())

    def test_computed_constants_array_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('computedConstants', g.computedConstantsArrayString())
        g.setComputedConstantsArrayString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.computedConstantsArrayString())

    def test_algebraic_variables_array_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('algebraicVariables', g.algebraicVariablesArrayString())
        g.setAlgebraicVariablesArrayString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.algebraicVariablesArrayString())

    def test_external_variables_array_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('externalVariables', g.externalVariablesArrayString())
        g.setExternalVariablesArrayString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.externalVariablesArrayString())

    def test_external_variable_method_type_definition_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual(
            'typedef double (* ExternalVariable)(double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, size_t index);\n',
            g.externalVariableMethodTypeDefinitionString(False))
        g.setExternalVariableMethodTypeDefinitionString(False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.externalVariableMethodTypeDefinitionString(False))

        self.assertEqual(
            'typedef double (* ExternalVariable)(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, size_t index);\n',
            g.externalVariableMethodTypeDefinitionString(True))
        g.setExternalVariableMethodTypeDefinitionString(True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.externalVariableMethodTypeDefinitionString(True))

    def test_external_variable_method_call_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('externalVariable(constants, computedConstants, algebraicVariables, externalVariables, [INDEX])',
                         g.externalVariableMethodCallString(False))
        g.setExternalVariableMethodCallString(False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.externalVariableMethodCallString(False))

        self.assertEqual('externalVariable(voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables, [INDEX])',
                         g.externalVariableMethodCallString(True))
        g.setExternalVariableMethodCallString(True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.externalVariableMethodCallString(True))

    def test_root_finding_info_object_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('typedef struct {\n    double *constants;\n    double *computedConstants;\n    double *algebraicVariables;\n} RootFindingInfo;\n', g.rootFindingInfoObjectString(False, False))
        g.setRootFindingInfoObjectString(False, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.rootFindingInfoObjectString(False, False))

        self.assertEqual('typedef struct {\n    double *constants;\n    double *computedConstants;\n    double *algebraicVariables;\n    double *externalVariables;\n} RootFindingInfo;\n', g.rootFindingInfoObjectString(False, True))
        g.setRootFindingInfoObjectString(False, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.rootFindingInfoObjectString(False, True))

        self.assertEqual('typedef struct {\n    double voi;\n    double *states;\n    double *rates;\n    double *constants;\n    double *computedConstants;\n    double *algebraicVariables;\n} RootFindingInfo;\n', g.rootFindingInfoObjectString(True, False))
        g.setRootFindingInfoObjectString(True, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.rootFindingInfoObjectString(True, False))

        self.assertEqual('typedef struct {\n    double voi;\n    double *states;\n    double *rates;\n    double *constants;\n    double *computedConstants;\n    double *algebraicVariables;\n    double *externalVariables;\n} RootFindingInfo;\n', g.rootFindingInfoObjectString(True, True))
        g.setRootFindingInfoObjectString(True, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.rootFindingInfoObjectString(True, True))

    def test_extern_nla_solve_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('extern void nlaSolve(void (*objectiveFunction)(double *, double *, void *),\n                     double *u, size_t n, void *data);\n', g.externNlaSolveMethodString())
        g.setExternNlaSolveMethodString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.externNlaSolveMethodString())

    def test_find_root_call_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('findRoot[INDEX](constants, computedConstants, algebraicVariables);\n', g.findRootCallString(False, False))
        g.setFindRootCallString(False, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.findRootCallString(False, False))

        self.assertEqual('findRoot[INDEX](constants, computedConstants, algebraicVariables, externalVariables);\n', g.findRootCallString(False, True))
        g.setFindRootCallString(False, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.findRootCallString(False, True))

        self.assertEqual('findRoot[INDEX](voi, states, rates, constants, computedConstants, algebraicVariables);\n', g.findRootCallString(True, False))
        g.setFindRootCallString(True, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.findRootCallString(True, False))

        self.assertEqual('findRoot[INDEX](voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables);\n', g.findRootCallString(True, True))
        g.setFindRootCallString(True, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.findRootCallString(True, True))

    def test_find_root_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('void findRoot[INDEX](double *constants, double *computedConstants, double *algebraicVariables)\n{\n    RootFindingInfo rfi = { constants, computedConstants, algebraicVariables };\n    double u[[SIZE]];\n\n[CODE]}\n', g.findRootMethodString(False, False))
        g.setFindRootMethodString(False, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.findRootMethodString(False, False))

        self.assertEqual('void findRoot[INDEX](double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables)\n{\n    RootFindingInfo rfi = { constants, computedConstants, algebraicVariables, externalVariables };\n    double u[[SIZE]];\n\n[CODE]}\n', g.findRootMethodString(False, True))
        g.setFindRootMethodString(False, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.findRootMethodString(False, True))

        self.assertEqual('void findRoot[INDEX](double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)\n{\n    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };\n    double u[[SIZE]];\n\n[CODE]}\n', g.findRootMethodString(True, False))
        g.setFindRootMethodString(True, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.findRootMethodString(True, False))

        self.assertEqual('void findRoot[INDEX](double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables)\n{\n    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables };\n    double u[[SIZE]];\n\n[CODE]}\n', g.findRootMethodString(True, True))
        g.setFindRootMethodString(True, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.findRootMethodString(True, True))

    def test_nla_solve_call_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('nlaSolve(objectiveFunction[INDEX], u, [SIZE], &rfi);\n', g.nlaSolveCallString(False, False))
        g.setNlaSolveCallString(False, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.nlaSolveCallString(False, False))

        self.assertEqual('nlaSolve(objectiveFunction[INDEX], u, [SIZE], &rfi);\n', g.nlaSolveCallString(False, True))
        g.setNlaSolveCallString(False, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.nlaSolveCallString(False, True))

        self.assertEqual('nlaSolve(objectiveFunction[INDEX], u, [SIZE], &rfi);\n', g.nlaSolveCallString(True, False))
        g.setNlaSolveCallString(True, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.nlaSolveCallString(True, False))

        self.assertEqual('nlaSolve(objectiveFunction[INDEX], u, [SIZE], &rfi);\n', g.nlaSolveCallString(True, True))
        g.setNlaSolveCallString(True, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.nlaSolveCallString(True, True))

    def test_objective_function_method_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('void objectiveFunction[INDEX](double *u, double *f, void *data)\n{\n    double *constants = ((RootFindingInfo *) data)->constants;\n    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;\n    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;\n\n[CODE]}\n', g.objectiveFunctionMethodString(False, False))
        g.setObjectiveFunctionMethodString(False, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.objectiveFunctionMethodString(False, False))

        self.assertEqual('void objectiveFunction[INDEX](double *u, double *f, void *data)\n{\n    double *constants = ((RootFindingInfo *) data)->constants;\n    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;\n    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;\n    double *externalVariables = ((RootFindingInfo *) data)->externalVariables;\n\n[CODE]}\n', g.objectiveFunctionMethodString(False, True))
        g.setObjectiveFunctionMethodString(False, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.objectiveFunctionMethodString(False, True))

        self.assertEqual('void objectiveFunction[INDEX](double *u, double *f, void *data)\n{\n    double voi = ((RootFindingInfo *) data)->voi;\n    double *states = ((RootFindingInfo *) data)->states;\n    double *rates = ((RootFindingInfo *) data)->rates;\n    double *constants = ((RootFindingInfo *) data)->constants;\n    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;\n    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;\n\n[CODE]}\n', g.objectiveFunctionMethodString(True, False))
        g.setObjectiveFunctionMethodString(True, False, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.objectiveFunctionMethodString(True, False))

        self.assertEqual('void objectiveFunction[INDEX](double *u, double *f, void *data)\n{\n    double voi = ((RootFindingInfo *) data)->voi;\n    double *states = ((RootFindingInfo *) data)->states;\n    double *rates = ((RootFindingInfo *) data)->rates;\n    double *constants = ((RootFindingInfo *) data)->constants;\n    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;\n    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;\n    double *externalVariables = ((RootFindingInfo *) data)->externalVariables;\n\n[CODE]}\n', g.objectiveFunctionMethodString(True, True))
        g.setObjectiveFunctionMethodString(True, True, GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.objectiveFunctionMethodString(True, True))

    def test_u_array_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('u', g.uArrayString())
        g.setUArrayString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.uArrayString())

    def test_f_array_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('f', g.fArrayString())
        g.setFArrayString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.fArrayString())

    def test_voi_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('voi', g.voiString())
        g.setVoiString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.voiString())

    def test_xor_function_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('double xor(double x, double y)\n{\n    return (x != 0.0) ^ (y != 0.0);\n}\n',
                         g.xorFunctionString())
        g.setXorFunctionString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.xorFunctionString())

    def test_xor_string(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertEqual('xor', g.xorString())
        g.setXorString(GeneratorProfileTestCase.VALUE)
        self.assertEqual(GeneratorProfileTestCase.VALUE, g.xorString())

    def test_has_interface(self):
        from libcellml import GeneratorProfile

        g = GeneratorProfile()

        self.assertTrue(g.hasInterface())
        g.setHasInterface(False)
        self.assertFalse(g.hasInterface())


if __name__ == '__main__':
    unittest.main()
