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

const sineModel = `<?xml version="1.0" encoding="UTF-8"?><model xmlns="http://www.cellml.org/cellml/2.0#" xmlns:cellml="http://www.cellml.org/cellml/2.0#" xmlns:xlink="http://www.w3.org/1999/xlink" name="sin"> \
  <component name="sin"> \
    <variable name="x" units="dimensionless" interface="public_and_private"/> \
    <variable units="dimensionless" name="sin" interface="public_and_private"/> \
    <math xmlns="http://www.w3.org/1998/Math/MathML"> \
      <apply><eq/> \
        <ci>sin</ci> \
        <apply><sin/> \
          <ci>x</ci> \
        </apply> \
      </apply> \
    </math> \
  </component> \
</model> \
`

const componentImportModel = `<?xml version="1.0" encoding="UTF-8"?><model xmlns="http://www.cellml.org/cellml/2.0#" xmlns:cellml="http://www.cellml.org/cellml/2.0#" xmlns:xlink="http://www.w3.org/1999/xlink" name="sin_approximations_import"> \
  <import xlink:href="sin.xml"> \
    <component name="actual_sin" component_ref="sin"/> \
  </import> \
</model> \
`


const modelWithError = `<?xml version="1.0" encoding="UTF-8"?>
<model name="initialised_variable_of_integration" xmlns="http://www.cellml.org/cellml/2.0#" xmlns:cellml="http://www.cellml.org/cellml/2.0#">
    <units name="per_second">
        <unit exponent="-1" units="second"/>
    </units>
    <component name="my_component">
        <variable name="time" units="second" initial_value="0"/>
        <variable name="x" units="dimensionless" initial_value="0"/>
        <math xmlns="http://www.w3.org/1998/Math/MathML">
            <apply>
                <eq/>
                <apply>
                    <diff/>
                    <bvar>
                        <ci>time</ci>
                    </bvar>
                    <ci>x</ci>
                </apply>
                <cn cellml:units="per_second">1</cn>
            </apply>
        </math>
    </component>
</model>
`

const basicModel = `<?xml version="1.0" encoding="UTF-8"?>
<model name="model" xmlns="http://www.cellml.org/cellml/2.0#" xmlns:cellml="http://www.cellml.org/cellml/2.0#">
    <units name="per_second">
        <unit exponent="-1" units="second"/>
    </units>
    <component name="component">
        <variable name="time" units="second"/>
        <variable name="x" units="dimensionless" initial_value="0"/>
        <math xmlns="http://www.w3.org/1998/Math/MathML">
            <apply>
                <eq/>
                <apply>
                    <diff/>
                    <bvar>
                        <ci>time</ci>
                    </bvar>
                    <ci>x</ci>
                </apply>
                <cn cellml:units="per_second">1</cn>
            </apply>
        </math>
    </component>
</model>
`

const duplicatedIdsModel = `<?xml version="1.0" encoding="UTF-8"?>
<model xmlns="http://www.cellml.org/cellml/2.0#" name="everything" id="duplicateId2" >
  <import xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="some-other-model.xml" id="duplicateId1">
    <component component_ref="a_component_in_that_model" name="component1" id="duplicateId4"/>
  </import>
  <import xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="some-other-model.xml" id="duplicateId3" >
    <units units_ref="a_units_in_that_model" name="units1" id="duplicateId2" />
  </import>
  <units name="units2" id="duplicateId1">
    <unit units="second" id="duplicateId4"/>
  </units>
  <units name="units3" id="duplicateId3"/>
  <units name="blob" id="duplicateId2"/>
  <component name="component2" id="duplicateId1">
    <variable name="variable1" units="units2" interface="private" id="duplicateId4"/>
    <variable name="variable2" units="units2" id="duplicateId3"/>
    <reset variable="variable1" test_variable="variable2" order="1" id="duplicateId2">
      <test_value id="duplicateId1">
        <math xmlns="http://www.w3.org/1998/Math/MathML" xmlns:cellml="http://www.cellml.org/cellml/2.0#">
          <apply>
            <eq/>
            <ci>variable1</ci>
            <cn cellml:units="units2">3.4</cn>
          </apply>
        </math>
      </test_value>
      <reset_value id="duplicateId4">
        <math xmlns="http://www.w3.org/1998/Math/MathML" xmlns:cellml="http://www.cellml.org/cellml/2.0#">
          <apply>
            <eq/>
            <ci>variable1</ci>
            <cn cellml:units="units2">9.0</cn>
          </apply>
        </math>
      </reset_value>
    </reset>
  </component>
  <component name="component3" id="duplicateId3">
    <variable name="variable1" units="units2" interface="public" id="duplicateId2" />
    <variable name="variable2" units="units2" interface="public" id="duplicateId1" />
    <math xmlns="http://www.w3.org/1998/Math/MathML" xmlns:cellml="http://www.cellml.org/cellml/2.0#">
      <apply>
        <eq/>
        <ci>variable4</ci>
        <cn cellml:units="units2">9.0</cn>
      </apply>
    </math>
  </component>
  <component name="component4" id="duplicateId4">
    <variable name="variable1" units="units2" interface="public" id="duplicateId3" />
    <variable name="variable2" units="units2" interface="public" id="duplicateId2" />
    <math xmlns="http://www.w3.org/1998/Math/MathML" xmlns:cellml="http://www.cellml.org/cellml/2.0#">
      <apply>
        <eq/>
        <ci>variable4</ci>
        <cn cellml:units="units2">9.0</cn>
      </apply>
    </math>
  </component>
  <connection component_1="component2" component_2="component3" id="duplicateId1">
    <map_variables variable_1="variable1" variable_2="variable1" id="duplicateId4"/>
    <map_variables variable_1="variable2" variable_2="variable2" id="duplicateId3"/>
  </connection>
  <connection component_1="component2" component_2="component4" id="duplicateId2">
    <map_variables variable_1="variable1" variable_2="variable1" id="duplicateId1"/>
    <map_variables variable_1="variable2" variable_2="variable2" id="duplicateId4"/>
  </connection>
  <encapsulation id="duplicateId3">
    <component_ref component="component2" id="duplicateId2">
      <component_ref component="component3" id="duplicateId1"/>
    </component_ref>
  </encapsulation>
</model>
`

const modelWithParseErrors = `<?xml version="1.0" encoding="UTF-8"?>
<model xmlns="http://www.cellml.org/cellml/2.0#">
  <import xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="some-other-model.xml" sauce="hollandaise">
    <units units_ref="a_units_in_that_model" name="units_in_this_model"/>
    <component component_ref="a_component_in_that_model" name="component_in_this_model"/>
    <invalid_nonsense/>
    <units units_ruff="dog" name="fido"/>
    <component component_meow="cat" name="frank"/>
  </import>
</model>
`

const hhSquidAxon1952 = `<?xml version='1.0' encoding='UTF-8'?>
<model name="hodgkin_huxley_squid_axon_model_1952" xmlns="http://www.cellml.org/cellml/2.0#" xmlns:cellml="http://www.cellml.org/cellml/2.0#">
    <units name="millisecond">
        <unit prefix="milli" units="second"/>
    </units>
    <units name="per_millisecond">
        <unit exponent="-1" prefix="milli" units="second"/>
    </units>
    <units name="millivolt">
        <unit prefix="milli" units="volt"/>
    </units>
    <units name="per_millivolt_millisecond">
        <unit exponent="-1" units="millivolt"/>
        <unit exponent="-1" prefix="milli" units="second"/>
    </units>
    <units name="milliS_per_cm2">
        <unit prefix="milli" units="siemens"/>
        <unit exponent="-2" prefix="centi" units="metre"/>
    </units>
    <units name="microF_per_cm2">
        <unit prefix="micro" units="farad"/>
        <unit exponent="-2" prefix="centi" units="metre"/>
    </units>
    <units name="microA_per_cm2">
        <unit prefix="micro" units="ampere"/>
        <unit exponent="-2" prefix="centi" units="metre"/>
    </units>
    <component name="environment">
        <variable interface="public" name="time" units="millisecond"/>
    </component>
    <component name="membrane">
        <variable initial_value="0" interface="public" name="V" units="millivolt"/>
        <variable initial_value="0" interface="public" name="E_R" units="millivolt"/>
        <variable initial_value="1" name="Cm" units="microF_per_cm2"/>
        <variable interface="public" name="time" units="millisecond"/>
        <variable interface="public" name="i_Na" units="microA_per_cm2"/>
        <variable interface="public" name="i_K" units="microA_per_cm2"/>
        <variable interface="public" name="i_L" units="microA_per_cm2"/>
        <variable name="i_Stim" units="microA_per_cm2"/>
        <math xmlns="http://www.w3.org/1998/Math/MathML">
            <apply>
                <eq/>
                <ci>i_Stim</ci>
                <piecewise>
                    <piece>
                        <apply>
                            <minus/>
                            <cn cellml:units="microA_per_cm2">20</cn>
                        </apply>
                        <apply>
                            <and/>
                            <apply>
                                <geq/>
                                <ci>time</ci>
                                <cn cellml:units="millisecond">10</cn>
                            </apply>
                            <apply>
                                <leq/>
                                <ci>time</ci>
                                <cn cellml:units="millisecond">10.5</cn>
                            </apply>
                        </apply>
                    </piece>
                    <otherwise>
                        <cn cellml:units="microA_per_cm2">0</cn>
                    </otherwise>
                </piecewise>
            </apply>
            <apply>
                <eq/>
                <apply>
                    <diff/>
                    <bvar>
                        <ci>time</ci>
                    </bvar>
                    <ci>V</ci>
                </apply>
                <apply>
                    <divide/>
                    <apply>
                        <minus/>
                        <apply>
                            <plus/>
                            <apply>
                                <minus/>
                                <ci>i_Stim</ci>
                            </apply>
                            <ci>i_Na</ci>
                            <ci>i_K</ci>
                            <ci>i_L</ci>
                        </apply>
                    </apply>
                    <ci>Cm</ci>
                </apply>
            </apply>
        </math>
    </component>
    <component name="sodium_channel">
        <variable interface="public" name="i_Na" units="microA_per_cm2"/>
        <variable initial_value="120" name="g_Na" units="milliS_per_cm2"/>
        <variable name="E_Na" units="millivolt"/>
        <variable interface="public_and_private" name="time" units="millisecond"/>
        <variable interface="public_and_private" name="V" units="millivolt"/>
        <variable interface="public" name="E_R" units="millivolt"/>
        <variable interface="private" name="m" units="dimensionless"/>
        <variable interface="private" name="h" units="dimensionless"/>
        <math xmlns="http://www.w3.org/1998/Math/MathML">
            <apply>
                <eq/>
                <ci>E_Na</ci>
                <apply>
                    <minus/>
                    <ci>E_R</ci>
                    <cn cellml:units="millivolt">115</cn>
                </apply>
            </apply>
            <apply>
                <eq/>
                <ci>i_Na</ci>
                <apply>
                    <times/>
                    <ci>g_Na</ci>
                    <apply>
                        <power/>
                        <ci>m</ci>
                        <cn cellml:units="dimensionless">3</cn>
                    </apply>
                    <ci>h</ci>
                    <apply>
                        <minus/>
                        <ci>V</ci>
                        <ci>E_Na</ci>
                    </apply>
                </apply>
            </apply>
        </math>
    </component>
    <component name="sodium_channel_m_gate">
        <variable initial_value="0.05" interface="public" name="m" units="dimensionless"/>
        <variable name="alpha_m" units="per_millisecond"/>
        <variable name="beta_m" units="per_millisecond"/>
        <variable interface="public" name="V" units="millivolt"/>
        <variable interface="public" name="time" units="millisecond"/>
        <math xmlns="http://www.w3.org/1998/Math/MathML">
            <apply>
                <eq/>
                <ci>alpha_m</ci>
                <apply>
                    <divide/>
                    <apply>
                        <times/>
                        <cn cellml:units="per_millivolt_millisecond">0.1</cn>
                        <apply>
                            <plus/>
                            <ci>V</ci>
                            <cn cellml:units="millivolt">25</cn>
                        </apply>
                    </apply>
                    <apply>
                        <minus/>
                        <apply>
                            <exp/>
                            <apply>
                                <divide/>
                                <apply>
                                    <plus/>
                                    <ci>V</ci>
                                    <cn cellml:units="millivolt">25</cn>
                                </apply>
                                <cn cellml:units="millivolt">10</cn>
                            </apply>
                        </apply>
                        <cn cellml:units="dimensionless">1</cn>
                    </apply>
                </apply>
            </apply>
            <apply>
                <eq/>
                <ci>beta_m</ci>
                <apply>
                    <times/>
                    <cn cellml:units="per_millisecond">4</cn>
                    <apply>
                        <exp/>
                        <apply>
                            <divide/>
                            <ci>V</ci>
                            <cn cellml:units="millivolt">18</cn>
                        </apply>
                    </apply>
                </apply>
            </apply>
            <apply>
                <eq/>
                <apply>
                    <diff/>
                    <bvar>
                        <ci>time</ci>
                    </bvar>
                    <ci>m</ci>
                </apply>
                <apply>
                    <minus/>
                    <apply>
                        <times/>
                        <ci>alpha_m</ci>
                        <apply>
                            <minus/>
                            <cn cellml:units="dimensionless">1</cn>
                            <ci>m</ci>
                        </apply>
                    </apply>
                    <apply>
                        <times/>
                        <ci>beta_m</ci>
                        <ci>m</ci>
                    </apply>
                </apply>
            </apply>
        </math>
    </component>
    <component name="sodium_channel_h_gate">
        <variable initial_value="0.6" interface="public" name="h" units="dimensionless"/>
        <variable name="alpha_h" units="per_millisecond"/>
        <variable name="beta_h" units="per_millisecond"/>
        <variable interface="public" name="V" units="millivolt"/>
        <variable interface="public" name="time" units="millisecond"/>
        <math xmlns="http://www.w3.org/1998/Math/MathML">
            <apply>
                <eq/>
                <ci>alpha_h</ci>
                <apply>
                    <times/>
                    <cn cellml:units="per_millisecond">0.07</cn>
                    <apply>
                        <exp/>
                        <apply>
                            <divide/>
                            <ci>V</ci>
                            <cn cellml:units="millivolt">20</cn>
                        </apply>
                    </apply>
                </apply>
            </apply>
            <apply>
                <eq/>
                <ci>beta_h</ci>
                <apply>
                    <divide/>
                    <cn cellml:units="per_millisecond">1</cn>
                    <apply>
                        <plus/>
                        <apply>
                            <exp/>
                            <apply>
                                <divide/>
                                <apply>
                                    <plus/>
                                    <ci>V</ci>
                                    <cn cellml:units="millivolt">30</cn>
                                </apply>
                                <cn cellml:units="millivolt">10</cn>
                            </apply>
                        </apply>
                        <cn cellml:units="dimensionless">1</cn>
                    </apply>
                </apply>
            </apply>
            <apply>
                <eq/>
                <apply>
                    <diff/>
                    <bvar>
                        <ci>time</ci>
                    </bvar>
                    <ci>h</ci>
                </apply>
                <apply>
                    <minus/>
                    <apply>
                        <times/>
                        <ci>alpha_h</ci>
                        <apply>
                            <minus/>
                            <cn cellml:units="dimensionless">1</cn>
                            <ci>h</ci>
                        </apply>
                    </apply>
                    <apply>
                        <times/>
                        <ci>beta_h</ci>
                        <ci>h</ci>
                    </apply>
                </apply>
            </apply>
        </math>
    </component>
    <component name="potassium_channel">
        <variable interface="public" name="i_K" units="microA_per_cm2"/>
        <variable initial_value="36" name="g_K" units="milliS_per_cm2"/>
        <variable name="E_K" units="millivolt"/>
        <variable interface="public_and_private" name="time" units="millisecond"/>
        <variable interface="public_and_private" name="V" units="millivolt"/>
        <variable interface="public" name="E_R" units="millivolt"/>
        <variable interface="private" name="n" units="dimensionless"/>
        <math xmlns="http://www.w3.org/1998/Math/MathML">
            <apply>
                <eq/>
                <ci>E_K</ci>
                <apply>
                    <plus/>
                    <ci>E_R</ci>
                    <cn cellml:units="millivolt">12</cn>
                </apply>
            </apply>
            <apply>
                <eq/>
                <ci>i_K</ci>
                <apply>
                    <times/>
                    <ci>g_K</ci>
                    <apply>
                        <power/>
                        <ci>n</ci>
                        <cn cellml:units="dimensionless">4</cn>
                    </apply>
                    <apply>
                        <minus/>
                        <ci>V</ci>
                        <ci>E_K</ci>
                    </apply>
                </apply>
            </apply>
        </math>
    </component>
    <component name="potassium_channel_n_gate">
        <variable initial_value="0.325" interface="public" name="n" units="dimensionless"/>
        <variable name="alpha_n" units="per_millisecond"/>
        <variable name="beta_n" units="per_millisecond"/>
        <variable interface="public" name="V" units="millivolt"/>
        <variable interface="public" name="time" units="millisecond"/>
        <math xmlns="http://www.w3.org/1998/Math/MathML">
            <apply>
                <eq/>
                <ci>alpha_n</ci>
                <apply>
                    <divide/>
                    <apply>
                        <times/>
                        <cn cellml:units="per_millivolt_millisecond">0.01</cn>
                        <apply>
                            <plus/>
                            <ci>V</ci>
                            <cn cellml:units="millivolt">10</cn>
                        </apply>
                    </apply>
                    <apply>
                        <minus/>
                        <apply>
                            <exp/>
                            <apply>
                                <divide/>
                                <apply>
                                    <plus/>
                                    <ci>V</ci>
                                    <cn cellml:units="millivolt">10</cn>
                                </apply>
                                <cn cellml:units="millivolt">10</cn>
                            </apply>
                        </apply>
                        <cn cellml:units="dimensionless">1</cn>
                    </apply>
                </apply>
            </apply>
            <apply>
                <eq/>
                <ci>beta_n</ci>
                <apply>
                    <times/>
                    <cn cellml:units="per_millisecond">0.125</cn>
                    <apply>
                        <exp/>
                        <apply>
                            <divide/>
                            <ci>V</ci>
                            <cn cellml:units="millivolt">80</cn>
                        </apply>
                    </apply>
                </apply>
            </apply>
            <apply>
                <eq/>
                <apply>
                    <diff/>
                    <bvar>
                        <ci>time</ci>
                    </bvar>
                    <ci>n</ci>
                </apply>
                <apply>
                    <minus/>
                    <apply>
                        <times/>
                        <ci>alpha_n</ci>
                        <apply>
                            <minus/>
                            <cn cellml:units="dimensionless">1</cn>
                            <ci>n</ci>
                        </apply>
                    </apply>
                    <apply>
                        <times/>
                        <ci>beta_n</ci>
                        <ci>n</ci>
                    </apply>
                </apply>
            </apply>
        </math>
    </component>
    <component name="leakage_current">
        <variable interface="public" name="i_L" units="microA_per_cm2"/>
        <variable initial_value="0.3" name="g_L" units="milliS_per_cm2"/>
        <variable name="E_L" units="millivolt"/>
        <variable interface="public" name="time" units="millisecond"/>
        <variable interface="public" name="V" units="millivolt"/>
        <variable interface="public" name="E_R" units="millivolt"/>
        <math xmlns="http://www.w3.org/1998/Math/MathML">
            <apply>
                <eq/>
                <ci>E_L</ci>
                <apply>
                    <minus/>
                    <ci>E_R</ci>
                    <cn cellml:units="millivolt">10.613</cn>
                </apply>
            </apply>
            <apply>
                <eq/>
                <ci>i_L</ci>
                <apply>
                    <times/>
                    <ci>g_L</ci>
                    <apply>
                        <minus/>
                        <ci>V</ci>
                        <ci>E_L</ci>
                    </apply>
                </apply>
            </apply>
        </math>
    </component>
    <encapsulation>
        <component_ref component="sodium_channel">
            <component_ref component="sodium_channel_m_gate"/>
            <component_ref component="sodium_channel_h_gate"/>
        </component_ref>
        <component_ref component="potassium_channel">
            <component_ref component="potassium_channel_n_gate"/>
        </component_ref>
    </encapsulation>
    <connection component_1="membrane" component_2="environment">
        <map_variables variable_1="time" variable_2="time"/>
    </connection>
    <connection component_1="leakage_current" component_2="environment">
        <map_variables variable_1="time" variable_2="time"/>
    </connection>
    <connection component_1="sodium_channel" component_2="environment">
        <map_variables variable_1="time" variable_2="time"/>
    </connection>
    <connection component_1="potassium_channel" component_2="environment">
        <map_variables variable_1="time" variable_2="time"/>
    </connection>
    <connection component_1="membrane" component_2="leakage_current">
        <map_variables variable_1="V" variable_2="V"/>
        <map_variables variable_1="E_R" variable_2="E_R"/>
        <map_variables variable_1="i_L" variable_2="i_L"/>
    </connection>
    <connection component_1="membrane" component_2="sodium_channel">
        <map_variables variable_1="V" variable_2="V"/>
        <map_variables variable_1="E_R" variable_2="E_R"/>
        <map_variables variable_1="i_Na" variable_2="i_Na"/>
    </connection>
    <connection component_1="membrane" component_2="potassium_channel">
        <map_variables variable_1="V" variable_2="V"/>
        <map_variables variable_1="E_R" variable_2="E_R"/>
        <map_variables variable_1="i_K" variable_2="i_K"/>
    </connection>
    <connection component_1="sodium_channel" component_2="sodium_channel_m_gate">
        <map_variables variable_1="m" variable_2="m"/>
        <map_variables variable_1="time" variable_2="time"/>
        <map_variables variable_1="V" variable_2="V"/>
    </connection>
    <connection component_1="sodium_channel" component_2="sodium_channel_h_gate">
        <map_variables variable_1="h" variable_2="h"/>
        <map_variables variable_1="time" variable_2="time"/>
        <map_variables variable_1="V" variable_2="V"/>
    </connection>
    <connection component_1="potassium_channel" component_2="potassium_channel_n_gate">
        <map_variables variable_1="n" variable_2="n"/>
        <map_variables variable_1="time" variable_2="time"/>
        <map_variables variable_1="V" variable_2="V"/>
    </connection>
</model>
`;

module.exports = {
    basicModel,
    componentImportModel,
    duplicatedIdsModel,
    hhSquidAxon1952,
    modelWithError,
    modelWithParseErrors,
    sineModel,
}
