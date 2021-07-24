#!/usr/bin/env bash

set -euo pipefail

cd $GITHUB_WORKSPACE

outname="lfc_nightly_$(date '+%Y%m%d-%H%M%S')"

# build lfc compiler
./gradlew generateStandaloneCompiler

# assemble the files in a separate directory
mkdir -p $outname/bin
mkdir -p $outname/lib

mv org.lflang/build/libs/org.lflang-*-SNAPSHOT-all.jar $outname/lib

# Move & patch wrappers
sed -e '/^lfbase=/d' -e 's/\${lfbase}build\/libs/\${base}lib/g' -e '/^if \[\[ ! -f "\$jarpath" \]\]; then/{n;N;N;d}' -e '/^if \[\[ ! -f "\$jarpath" \]\]; then/a \
    fatal_error "$jarpath does not exist."\
    error_exit' bin/lfc > $outname/bin/lfc
chmod u+x $outname/bin/lfc
sed -e '/^\$lfbase=/d' -e 's/\$lfbase\\build\\libs/\$base\\lib/g' -e '/^if (-not (Test-Path \$jarpath -PathType leaf))/{n;N;N;N;d}' -e '/^if (-not (Test-Path \$jarpath -PathType leaf))/a \
    throw "\$jarpath does not exist."' bin/lfc.ps1 > $outname/bin/lfc.ps1

# TODO: Move & patch ps1 wrapper script!

# zip/tar everything - the files will be put into the build_upload directory
mkdir -p build_upload
zip -r build_upload/$outname.zip $outname
tar cvf build_upload/$outname.tar.gz $outname
