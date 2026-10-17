# Scientific model packages

The reference/ descriptors are real generated synthetic models. Their source
digests correspond to the shipped reference engine file. Regenerate with make schemas.
A package binds a scientific model to a pinned engine/context; it is not the engine.

Future molecular/cellular/tissue/organ/system/organism/composite packages must pin
source, license, parameters, dependencies, ports and validity context. A composite
also pins every coupling model. No model here is a complete digital twin.

`native/` contains exact declarative SBML and admission metadata for an original
vendored PKML model, with separate native-agent plans. These do not register as
reference-engine models or confer biological validity; see native/README.md.

`cellml/` adds an original synthetic CellML 2 decay verification fixture and explicit
original-time-unit solver controls. It is not biological validation. Upstream
Robertson/unit-scaling examples remain pristine in vendor/dependencies/libcellml.
