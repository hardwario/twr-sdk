set -eu

einfo() {
    printf '\033[1;36m> %s\033[0m\n' "$@" >&2
}

einfo 'Checking job number...'
[ "${TRAVIS_JOB_NUMBER}" = "${TRAVIS_BUILD_NUMBER}.1" ] || {
    einfo 'This is not the first job.'
    einfo "Current job number is: ${TRAVIS_JOB_NUMBER}"
    einfo 'Exiting...'
    exit 0
}

einfo 'Deleting old content...'
rm -rf ${HTML_DIR}
mkdir -p ${HTML_DIR}

einfo 'Creating .nojekyll file...'
echo '' > ${HTML_DIR}/.nojekyll

einfo 'Generating Doxygen documentation...'
doxygen ${DOXYFILE} 2>&1 | tee doxygen.log

einfo 'Switching to HTML directory...'
cd ${HTML_DIR}

einfo 'Populating CNAME (if exists)...'
if [ "${GH_PAGES_CNAME:-}" ]; then
    echo "${GH_PAGES_CNAME}" > CNAME
fi
