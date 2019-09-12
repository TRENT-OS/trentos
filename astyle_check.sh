#!/bin/bash

cd "$(dirname "$0")"

ASTYLE_SETTINGS_LINUX_USER_SPACE="--suffix=none \
                                    --style=allman \
                                    --indent=spaces=4 \
                                    --indent-classes \
                                    --indent-namespaces \
                                    --pad-oper \
                                    --pad-header \
                                    --pad-comma \
                                    --add-brackets \
                                    --align-pointer=type \
                                    --align-reference=name \
                                    --min-conditional-indent=0 \
                                    --lineend=linux \
                                    --max-code-length=80 \
                                    --max-continuation-indent=60"

ASTYLE_SETTINGS_LINUX_KERNEL_SPACE="--style=1tbs \
                                    --indent=tab \
                                    --align-pointer=name \
                                    --add-brackets \
                                    --max-code-length=80"

# Alter this line to change configuration
ASTYLE_PARAMETERS=${ASTYLE_SETTINGS_LINUX_USER_SPACE}

if [ ! -z "$1" ] && [ $1 = "--help" ]; then
    echo "If you run the script without arguments then the files which are new or modified since the creation of the branch will be checked."
    echo "Otherwise you can use the argument list of this script to specify the files you want to check."
    echo "e.g.: ./astyle_check.sh \`git status -s | cut -c4- | grep -i '\.c$\|\.cpp$\|\.hpp$\|\.h$'\`"
    exit 0
fi

ASTYLE=astyle

case $(${ASTYLE} --version 2> /dev/null) in
  Artistic*)
      ;;
  default)
      echo "Did not find astyle, please install it before continuing."
      exit 1
      ;;
esac

RETVAL=0
files=$@

if [ -z "$files" ]; then

    # check any modified or new files. Note that there are many ways to get a
    # list of changes, but all have subtle differents. We are interested in
    # files from the current module only and don't care about submodules, so
    # the current command works good enough. If we need the changed submodules
    # included also then "git status --porcelain=v1 | cut -c4-" is the
    # better choice. However, there is no command line option available that
    # dives into the submodule and list the actualy files with changes.
    files=$(git ls-files --modified --others | grep -i '\.c$\|\.cpp$\|\.hpp$\|\.h$')

    # check all file that have been create or modified since branch creation
    files+=" "$(git diff-index --diff-filter=ACMR --name-only -r --cached origin/master | grep -i '\.c$\|\.cpp$\|\.hpp$\|\.h$')

fi


# sort and remove duplicates
files=$(echo ${files} | xargs -n1 | sort -u | xargs)

for file in ${files}; do
    OUT_FILE="${file}.astyle"

    ${ASTYLE} ${ASTYLE_PARAMETERS} <${file} >${OUT_FILE}
    diff ${file} ${file}.astyle > /dev/null
    if [ $? -ne 0 ]; then
        RETVAL=1
        echo "astyle error: ${file}"
        # what does this do ???
        ${ASTYLE} ${ASTYLE_PARAMETERS} ${OUT_FILE}
    else
        # everything ok, delete astyle file
        rm ${OUT_FILE}
    fi
done

exit $RETVAL
