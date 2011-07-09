#!/usr/bin/env bash

base=$BASH_SOURCE
base=`dirname "$base"`
base=`readlink -f "$base"`'/..'
test_base=`readlink -f "$base"`

PATH="$test_base/test:$PATH"

./czm "$test_base/data/CZ100404a" <"$test_base/test/czm.in" >czm.out.tmp
diff "$test_base/test/czm.out" czm.out.tmp
#exit $?
