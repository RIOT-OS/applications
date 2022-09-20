# Archival notice

The applications that resided in this repository have been [moved into RIOT].
Please see the [examples inside RIOT] to find both these and other applications.

The repository is left online only for documentation of the applications' history.

[moved into RIOT]: https://github.com/RIOT-OS/RIOT/pull/18602
[examples inside RIOT]: https://github.com/RIOT-OS/RIOT/tree/master/examples

---

[![Build Status](https://travis-ci.org/RIOT-OS/applications.svg?branch=master)](https://travis-ci.org/RIOT-OS/applications)

# RIOT Applications

This repository provides more applications for the [RIOT operating system][riot-repo].
Some of them are just useful tools for development work, others showcase
more extensive implementations of features of RIOT compared to the rather simple
[examples in the RIOT main codebase][riot-repo/examples].

## Usage

To build and use them follow [the instructions in the RIOT repository][getting-started]
and the READMEs within the respective application directory.

The RIOT main code is included as a submodule. This always points to the latest
release. To change the RIOT version to build against (e.g. current master),
clone the RIOT repository in a separate repository and point the `RIOTBASE`
environment variable there:

```sh
# assuming you are in the working directory of your local clone of this repo
cd ..
git clone git@github.com:RIOT-OS/RIOT.git
cd applications
RIOTBASE="../RIOT" BOARD=samr21-xpro make -C sniffer flash
```

Alternatively you can use RIOT as a submodule. To initialize the submodule, from the
root of the repository run:

```sh
git submodule update --init --recursive
```

If you want to use master then simply step into the submodule and checkout master or
any other desired branch.

```sh
cd RIOT
git checkout master
git pull
```

Note that there is no guarantee that it will build or work, since we only test
this repository against the latest release.

[riot-repo]: https://github.com/RIOT-OS/RIOT
[riot-repo/examples]: https://github.com/RIOT-OS/RIOT/tree/master/examples
[getting-started]: https://github.com/RIOT-OS/RIOT/blob/master/README.md#getting-started
