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
    echo "If you run the script without arguments only the files involved in the last commit are checked."
    echo "Otherwise you can use the argument list of this script to specify the files you want to check."
    echo "e.g.: ./astyle_check.sh \`git status -s | cut -c4- | grep -i '\.c$\|\.cpp$\|\.hpp$\|\.h$'\`"
    exit 0
fi

against=HEAD~1

ASTYLE=astyle

case `$ASTYLE --version 2> /dev/null` in
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
    files=$(git diff-index --diff-filter=ACMR --name-only -r --cached $against -- | grep -i '\.c$\|\.cpp$\|\.hpp$\|\.h$')
fi

for file in $files; do
    x=`echo $file`
    OUT_FILE=$file.astyle
    if [ ! -z "$x" ]; then
        $ASTYLE ${ASTYLE_PARAMETERS} <$file >"$OUT_FILE"
        diff $file "$file.astyle" > /dev/null
        if [ $? != 0 ]; then
            RETVAL=1
            echo "File $file is not style compliant"
            $ASTYLE ${ASTYLE_PARAMETERS} "$OUT_FILE"
        else
            rm "$OUT_FILE"
        fi
    fi
done

exit $RETVAL
