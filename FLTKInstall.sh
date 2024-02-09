#!/bin/bash
# 
#   FLTKInstall.sh: Install FLTK from GitHub
# 

#---if systemd logger operating use it
if [ -z "$LOG" ]; then export LOG=~/build.log; fi
if echo | logger > /dev/null 2>&1;
then
    function LOGGER() { logger --tag "[$0:$LN `date`]" -s 2>&1 | tee -a $LOG; }
else
    function LOGGER() { read LOGMSG && echo "[$0:$LN `date`]" $LOGMSG | tee -a $LOG; }
fi

WGET=wget;
FUNCTION=$1; shift 1
while getopts ":x:q" o; do
    case "${o}" in
    q)
      WGET="wget -q";       #   pass '-q', quiet, to the wget command
      ;;
    x)
       CONFXML=${OPTARG};   #   Configuration file in XML format, j:JSON, y:YAML, i:INI can be added later
      ;;
    ?)
      echo "Invalid option: -${OPTARG}."; exit 1
      ;;
  esac
done

#   Define $INSTALL. System/OS package installer
function installer() {
    INSTALLERS=(apt yum yast);
    for (( i=0; i<${#INSTALLERS[@]}; i++ )); do
        I="${INSTALLERS[i]}";
        if $I --version >/dev/null 2>&1; then INSTALL="${INSTALLERS[i]} install -y ${PACKAGES[i]}"; break; fi;
    done
}

#   Check for command, install if not available
function check_function() {
    if $1 --version 2>/dev/null; then return; fi;
    installer; 
    if sudo $INSTALL;
        then echo "--- \"$INSTALL\": Success  ---" | LOGGER;
        else echo "--- \"$INSTALL\": FAILURE! ---" | LOGGER; exit 1; fi;
}

#   Clone FLTK repo
#   NOTE:   FLTK repo git may not be practical!
#           Unless you intend to develop and/or switch branches, it's better to wget the ZIP with wget_fltk()
function git_fltk() {
    PACKAGES=(git git git); check_function "git";
    mkdir -p $REPODIR && cd $REPODIR && NSRC=${SRC##*/} && DIR=${NSRC%.*}
    if [ -d $DIR ]; then
        cd $DIR && echo "--- $DIR at branch: `git branch | head -1` ---" | LOGGER; return; fi;
    export GIT_HTTP_MAX_REQUEST_BUFFER=2000M && export GIT_HTTP_CLONE_TIMEOUT=200m
    if git clone --depth 1 $SRC --recursive -v;
        then echo "--- git clone: Success  ---" | LOGGER;
        else echo "--- git clone: FAILURE! ---" | LOGGER; exit 1; fi;
}

#   Checkout FLTK branch
function git_checkout() {
    git_fltk;
    if git checkout $FLTKVER;
        then echo "--- git checkout $FLTKVER: Success  ---" | LOGGER;
        else echo "--- git checkout $FLTKVER: FAILURE! ---" | LOGGER; exit 1; fi;
}

#   WGET FLTK branch
function wget_fltk() {
    PACKAGES=("wget unzip" "wget unzip" "wget unzip"); check_function "wget";
    mkdir -p $REPODIR && NSRC=${SRC##*/} && DIR=${NSRC%.*}
    if $WGET $FLTKZIP -O $REPODIR/$DIR.zip;
        then echo "--- $WGET $FLTKZIP: Success  ---" | LOGGER;
        else echo "--- $WGET $FLTKZIP: FAILURE! ---" | LOGGER; exit 1; fi;
    if cd $REPODIR && unzip -o $DIR.zip && rm $DIR.zip;
        then echo "--- unzip FLTK: Success  ---" | LOGGER;
        else echo "--- unzip FLTK: FAILURE! ---" | LOGGER; exit 1; fi;
}

#   Configure with CMake.
function config_fltk() {
    PACKAGES=(cmake cmake cmake); check_function "cmake"; pwd
    NSRC=${SRC##*/} && DIR=${NSRC%.*}; pwd
    if mkdir -p $REPODIR && cd $REPODIR &&\
       cd $DIR-branch-$FLTKBRANCH && mkdir -p build && cd build;
        then echo "--- Create build dir: Success  ---" | LOGGER;
        else echo "--- Create build dir: FAILURE! ---" | LOGGER; exit 1; fi;
    if cmake $CMAKEFLAGS ../;
        then echo "--- cmake "$CMAKEFLAGS": Success  ---" | LOGGER;
        else echo "--- cmake "$CMAKEFLAGS": FAILURE! ---" | LOGGER; exit 1; fi;
}
#   Version.
function version() {
    if ! cd $REPODIR/fltk-branch-$FLTKBRANCH/build;
        then echo "--- Build directory not available! ---" | LOGGER; exit 1; fi;
    if VER=`./bin/fltk-config --version`;
        then echo "--- FLTK Version: $VER  ---" | LOGGER;
        else echo "--- FLTK Version: FAILURE! ---" | LOGGER; exit 1; fi;
}

#   Make
function build() {
    mkdir -p $REPODIR && cd $REPODIR && NSRC=${SRC##*/} && DIR=${NSRC%.*}
    cd $DIR-branch-$FLTKBRANCH; mkdir -p build; cd build
    if ! [ -f fltk-config ];then echo "--- FLTK source not configured! ---" | LOGGER; exit 1; fi;

    if make $MAKEFLAGS;
        then echo "--- make: Success  ---" | LOGGER;
        else echo "--- make: FAILURE! ---" | LOGGER; exit 1; fi;
}

#   Install
function install() {
    build;
    if sudo make $MAKEFLAGS install;
        then echo "--- make install: Success  ---" | LOGGER;
        else echo "--- make install: FAILURE! ---" | LOGGER; exit 1; fi;
}

#---End of function declarations----------------------------------------------

# 
#   Read configuration
# 
if ! [ -z "${CONFXML}" ]; then
    check_xmllint;
    PACKAGES=(libxml2-utils xmlstarlet xmlstarlet); check_function "xmllint";
    CNT=`xmllint --xpath 'count(/config/*)' $CONFXML`
    for ((i=1; i<=$CNT; i++)); do
        PARAM=`xmllint --xpath "name(/config/*[$i])" $CONFXML`
        VALUE=`xmllint --xpath "string(/config/*[$i])" $CONFXML`
        declare $PARAM="$VALUE";
    done
fi

if [ -z "${FLTKBRANCH}" ];  then FLTKBRANCH=1.3; fi
if [ -z "${SRC}" ];         then SRC=https://github.com/fltk/fltk.git; fi
if [ -z "${FLTKZIP}" ];     then FLTKZIP=https://github.com/fltk/fltk/archive/refs/heads/branch-$FLTKBRANCH.zip; fi
if [ -z "${REPODIR}" ];     then REPODIR=repo; fi
if [ -z "${PREFIX}" ];      then PREFIX=/usr/local; fi
if [ -z "${MAKEFLAGS}" ];   then MAKEFLAGS=-j; fi
if [ -z "${CMAKEFLAGS}" ];  then CMAKEFLAGS="-D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX="$PREFIX" -D OPTION_BUILD_SHARED_LIBS=ON -D CMAKE_C_FLAGS=-fPIC -D CMAKE_CXX_FLAGS=-fPIC"; fi

#   OS and architecture
if [ -z "${OS}" ];          then OS=Linux; fi
if [ -z "${ARCH}" ];        then ARCH=x86-64; fi

$FUNCTION