[![Build Status](https://travis-ci.org/RIOT-OS/applications.svg?branch=master)](https://travis-ci.org/RIOT-OS/applications)

This repository provides some applications for the [RIOT operating system][riot-repo]. Some of them are just useful tools for
development work, some showcase software provided with RIOT beyond the simple
examples within the RIOT codebase.

To build and use them follow [the instructions in the RIOT repository][getting-started]
and the respective application's README.

The submodule to the RIOT repository in this repository will always point to the
latest release. To change the RIOT version to build against (e.g. current
master), clone the RIOT repository in a separate repository and point the
`RIOTBASE` environment variable there:

```sh
# assuming you are in the working directory of your local clone of this repo
cd ..
git clone git@github.com:RIOT-OS/RIOT.git
cd applications
RIOTBASE="../RIOT" BOARD=samr21-xpro make -C sniffer flash
```

[riot-repo]: https://github.com/RIOT-OS/RIOT
[getting-started]: https://github.com/RIOT-OS/RIOT/blob/master/README.md#getting-started
