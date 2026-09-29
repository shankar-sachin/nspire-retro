# Security Policy

## Supported Versions

Security fixes are applied to the latest development version and the newest released version.

| Version | Supported |
|---|---|
| Latest development branch | Yes |
| Latest release | Yes |
| Older releases | No |

## Reporting a Vulnerability

Please do not disclose security issues publicly before they are fixed.

Report vulnerabilities through GitHub’s private security advisory feature:

https://github.com/shankar-sachin/nspire-retro/security/advisories/new

Include:

- A clear description of the issue
- Steps to reproduce it
- The affected version or commit
- Any relevant save file, input sequence, or proof of concept
- The possible impact

You should receive an acknowledgement within 14 days. We will investigate, confirm the issue, and coordinate a fix or disclosure timeline.

## Scope

Security reports are especially useful for:

- Save-file parsing or validation vulnerabilities
- Buffer overflows, memory corruption, or undefined behavior
- Malformed `.tns` or save files causing crashes
- Issues that bypass salary-cap, draft, or career-state validation
- Build or release-process vulnerabilities
- Supply-chain issues involving the Ndless toolchain or repository

The landing page is static and does not collect accounts, passwords, payment data, or personal information.

## Out of Scope

The following are generally not security vulnerabilities:

- Missing gameplay features
- Balance issues
- Fictional team or player ratings
- Unverified calculator performance
- Bugs requiring physical access to a calculator
- Issues in unsupported older releases
- Suggestions for stronger passwords or account security on GitHub itself

## Save Files and Personal Data

Nspire Retro saves are local files stored beside the calculator program. Do not include personal information in bug reports or public issues. Remove or anonymize save files before sharing them unless they are necessary to reproduce a vulnerability.

## Responsible Disclosure

Please allow reasonable time for investigation and remediation before public disclosure. We will credit reporters who help responsibly, unless they prefer to remain anonymous.
