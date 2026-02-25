# Certifiable Build

This repository contains the build infrastructure that supports the [build2](https://www.build2.org) tools for developers and CI systems like GitHub Actions for projects in this organization.

The repository is included via `git subtree` in the individual component repositories.

## Building Components

All components are built using the same four step process:

1. `make setup` installs the compilers and build2 tools;
1. `make config` configures the source code for building;
1. `make build` builds the source code into artifacts;
1. `make test` runs the tests for the component.

If you wish, you can skip the `make setup` step and install the required C/C++ compilers and build2 tools yourself.

## License

**Dual Licensed:**

* **Open Source:** GNU General Public License v3.0 (GPLv3)
* **Commercial:** Available for proprietary use in safety-critical systems

For commercial licensing and compliance documentation packages, contact below.

## Patent Protection

This implementation is built on the **Murray Deterministic Computing Platform (MDCP)**,
protected by UK Patent **GB2521625.0**.

For commercial licensing inquiries: william@fstopify.com

## About

Built by **SpeyTech** in the Scottish Highlands.

30 years of UNIX infrastructure experience applied to deterministic computing for safety-critical systems.

Patent: UK GB2521625.0 - Murray Deterministic Computing Platform (MDCP)

**Contact:**
William Murray
william@fstopify.com
[speytech.com](https://speytech.com)

---

*Building deterministic AI systems for when lives depend on the answer.*

Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
