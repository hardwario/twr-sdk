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

einfo 'Retrieving commit author...'
commit_author="$(git log -n 1 --format='%an <%ae>' ${TRAVIS_COMMIT})"

einfo 'Retrieving commit date...'
commit_date="$(git log -n 1 --format='%aD' ${TRAVIS_COMMIT})"

einfo "Cloning branch ${GH_PAGES_BRANCH} from https://${GH_REPO_REF}..."
git clone -b ${GH_PAGES_BRANCH} https://${GH_REPO_REF} ${HTML_DIR}

einfo 'Deleting old content...'
git -C ${HTML_DIR} rm -rf --ignore-unmatch .

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

einfo 'Adding files to staging area...'
git add --all

einfo 'Checking if changes exist...'
[ -n "$(git status --porcelain)" ] || {
    einfo 'No changes to be commited - exiting...'
    exit 0
}

einfo 'Commiting changes...'
git commit --author="${commit_author}" --date="${commit_date}" -F- <<-EOF
    Deploy Doxygen content to GitHub Pages

    Travis CI build number: ${TRAVIS_BUILD_NUMBER}
    Content generated from: ${TRAVIS_COMMIT}
EOF

einfo "Pushing branch ${GH_PAGES_BRANCH} to https://${GH_REPO_REF}..."
git push --force "https://${GH_REPO_TOKEN}@${GH_REPO_REF}" ${GH_PAGES_BRANCH} > /dev/null 2>&1
