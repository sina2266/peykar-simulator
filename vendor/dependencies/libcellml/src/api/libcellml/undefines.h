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

// Note that:
//  1) this header file MUST NOT start with #pragma once; and that
//  2) this header file MUST be included LAST by a .cpp/.h file.
//
// This ensures that TRUE, FALSE, and NAN can be safely used in AnalyserEquationAst::Type.

#ifdef TRUE
#    undef TRUE
#endif

#ifdef FALSE
#    undef FALSE
#endif

#ifdef NAN
#    undef NAN
#endif
