#!/usr/bin/env bash

base=$BASH_SOURCE
base=`dirname "$base"`
base=`readlink -f "$base"`'/..'
test_base=`readlink -f "$base"`

PATH="$test_base/test:$PATH"

./czm "$test_base/data/CZ100404a" <"$test_base/test/czm_test.in" >czm_test.out.tmp
diff "$test_base/test/czm_test.out" czm_test.out.tmp
#exit $?
