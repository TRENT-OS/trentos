#!/bin/sh

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

against=HEAD~1

# If you want to allow non-ascii filenames set this variable to true.
allownonascii=$(git config hooks.allownonascii)

# Cross platform projects tend to avoid non-ascii filenames; prevent
# them from being added to the repository. We exploit the fact that the
# printable range starts at the space character and ends with tilde.
if [ "$allownonascii" != "true" ] &&
    # Note that the use of brackets around a tr range is ok here, (it's
    # even required, for portability to Solaris 10's /usr/bin/tr), since
    # the square bracket bytes happen to fall in the designated range.
    test "$(git diff --cached --name-only --diff-filter=A -z $against |
      LC_ALL=C tr -d '[ -~]\0')"
then
    echo "Error: Attempt to add a non-ascii file name."
    echo
    echo "This can cause problems if you want to work"
    echo "with people on other platforms."
    echo
    echo "To be portable it is advisable to rename the file ..."
    echo
    echo "If you know what you are doing you can disable this"
    echo "check using:"
    echo
    echo "  git config hooks.allownonascii true"
    echo
    exit 1
fi

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

files=$(git diff-index --diff-filter=ACMR --name-only -r --cached $against -- | grep -i '\.c$\|\.cpp$\|\.hpp$\|\.h$')
for file in $files; do
    x=`echo $file`
    if test "x$x" != "x"; then
        $ASTYLE ${ASTYLE_PARAMETERS} <$file >"$file.astyle"
        diff $file "$file.astyle" > /dev/null
        if [ $? != 0 ]; then
            RETVAL=1
            echo "File $file is not style compliant"
            # Remove trailing spaces
            sed -i 's/[[:space:]]*$//' "$file.astyle"
            $ASTYLE ${ASTYLE_PARAMETERS} "$file.astyle"
        else
            rm "$file.astyle"
        fi
    fi
done

exit $RETVAL
