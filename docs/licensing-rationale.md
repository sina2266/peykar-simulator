# Licensing rationale and review record

Prepared 2026-10-15 for the owner's requested noncommercial policy. This explains a
concrete draft, not a legal opinion about enforceability in every country.

## Why the previous license changed

Apache-2.0 permits commercial use and redistribution, subject to its terms. It does
not meet the owner's current wish to reserve commercial use. Any earlier valid
grants remain effective; changing a file cannot revoke them. The exact baseline is
recorded in `licenses/baseline-0.1.0.json`, and its license is retained in full.

## Alternatives considered

| Option | Relevant property | Decision for the stated policy |
|---|---|---|
| Apache-2.0 / MIT / BSD | Commercial use is permitted | Cannot impose the requested ban |
| GPL / AGPL | Copyleft obligations, not a commercial-use prohibition | Not a ban on charging or commercial use |
| PolyForm Noncommercial 1.0.0 | Standardized software license with broad allowed-organization provisions | Closest standard option, but not an exact blanket ban on paid organizational uses |
| PolyForm Strict 1.0.0 | Removes redistribution/modification permissions | Poor fit for collaborative forks |
| CC BY-NC variants | Creative Commons discourages CC licenses for software | Not chosen for the software |
| Peykar Noncommercial Source License 1.1.0 | Custom purpose and paid-service definitions | Draft prepared to match the stricter request; legal review required |

In particular, PolyForm Noncommercial treats use by listed charitable, educational,
public research, health/safety, environmental and governmental organizations as a
permitted purpose irrespective of funding or related obligations. That text should
not be described as a blanket prohibition of all paid or commercial arrangements.
The Peykar draft uses no unconditional nonprofit-entity exemption: **the activity
must itself be noncommercial**.

A noncommercial restriction is not generally compatible with distributing the
same combined derivative under an unmodified GPL/AGPL grant. A project being
open-source does not by itself authorize that combination or commercial use.
Preserve dependencies' terms and have the actual integration reviewed.

The draft is independently labeled and is not represented as a standard PolyForm
license, OSI-approved license, or pre-reviewed substitute for legal advice.

## Examples for interpreting the draft

- A student runs simulations for personal study and shares a free fork: allowed
  within LICENSE, including the attribution/source conditions.
- A university conducts genuinely public-benefit, noncommercial research with a
  general academic grant: funding alone does not make it prohibited.
- A university sells company-specific simulations or delivers sponsor-exclusive
  product-development results: outside the public grant for covered material.
- A charity charges for access to a Peykar-powered API: outside the public grant,
  despite nonprofit status.
- A business runs Peykar internally for development of a commercial therapy:
  outside the public grant for covered material, even if it never sells software.
- A company already obtained valid Apache rights to the 0.1.0 baseline: those
  permissions cannot be withdrawn by this new license if it complies with their terms.
- A third party reimplements an unprotected idea without copying protected code:
  the existence of LICENSE alone does not establish infringement.

## What enforcement still requires

Revision 1.1.0 expressly permits genuine humanitarian services supplied free to
recipients, including ordinary salaries funded by unconditional donations/grants.
It excludes tied donations, paid privileged access and commercial deliverables.
Section 7A clarifies that the public restriction does not bind the owner's exercise
of rights he owns. A separate legal entity still needs an ownership basis or grant,
and contributor/third-party rights are not transferred by this clarification.

Identify the exact copied/used version and copyrighted expression; establish the
claimant's rights and any contributor/employment rights; identify which license
applies; preserve evidence of the actual restricted activity; and obtain advice
about notice, cure, remedies and jurisdiction. Avoid claiming ownership of all
AI-generated material merely because a copyright line was added. Legal treatment
of human contributions and AI-assisted output depends on applicable law. The U.S.
Copyright Office material below is a U.S. source, not a determination of Dutch law.

The draft does not fix damages, guarantee an injunction, invalidate mandatory legal
exceptions, or erase the previous Apache grant. The independent scientific validation
requirements are unrelated to the software's licensing restrictions.

## Primary sources consulted

- Open Source Definition, especially sections 1 and 6:
  https://opensource.org/osd
- Apache License 2.0, especially sections 2 and 4:
  https://www.apache.org/licenses/LICENSE-2.0
- PolyForm Noncommercial 1.0.0, complete operative text:
  https://polyformproject.org/licenses/noncommercial/1.0.0
- PolyForm Strict 1.0.0, complete operative text:
  https://polyformproject.org/licenses/strict/1.0.0
- Creative Commons FAQ, software licensing discussion:
  https://creativecommons.org/faq/#can-i-apply-a-creative-commons-license-to-software
- Dutch software-authorship guidance:
  https://www.auteursrecht.nl/ik-ben-maker/Software-programmeur
- U.S. Copyright Office, AI/human authorship report announcement:
  https://www.copyright.gov/newsnet/2025/1060.html
