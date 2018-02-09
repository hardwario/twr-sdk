#!/bin/bash
set -eu

fix_h() {
    # convert tab to spaces
    sed -r -i 's/\t/    /g' "$1"

    # endif comment
    sed -r -i 's/^#endif \/\*\s*(.*?)\s*\*\/\s*$/#endif \/\/ \1/g' "$1"

    # return bool first letter big
    sed -r -i 's/(\/\/! @return (true|false) )([a-z])/\1\u\3/g' "$1"

    # dummy space
    sed -r -i 's/(.*?)\s+$/\1/g' "$1"
}

find bcl -name "*.h" | while read file; do
    fix_h "$file";
done

