from subprocess import check_output
from datetime import datetime

try:
    version=check_output(['git', 'describe', '--tags', '--abbrev=0', '--dirty=m']).decode().strip()
except:
    version='vdev'

try:
    git_version=check_output(['git', 'describe', '--abbrev=8', '--always', '--tags', '--dirty= (modified)']).decode().strip()
except:
    git_version='?'

build_date=datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S UTC')

print('-D VERSION="\\"%s\\"" -D GIT_VERSION="\\"%s\\"" -D BUILD_DATE="\\"%s\\""' % (version, git_version, build_date))
